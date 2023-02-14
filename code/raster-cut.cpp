#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_utils.h"
#include "ogr_api.h"
#include "gdalwarper.h"

#include "geotypes-base.h"

#include <stdio.h>
#include <time.h>

inline int
Clamp(int Target, int MinValue, int MaxValue)
{
    int Result = Min(Max(Target, MinValue), MaxValue);
    return Result;
}

inline int
CoordToPixel(double TargetCoord, double FirstCoord, double PixelSize)
{
    int Pixel = (int)((TargetCoord - FirstCoord) / PixelSize);
    return Pixel;
}

static OGRGeometryH
XYGeomToPLGeom(v2* InGeom, int NumPoints, double* SrcAffine, int NumRows, int NumCols)
{
    OGRGeometryH Ring = OGR_G_CreateGeometry(wkbLinearRing);
    
    for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        v2 Point = InGeom[PointIdx];
        int XPixel = CoordToPixel(Point.X, SrcAffine[0], SrcAffine[1]);
        int YPixel = CoordToPixel(Point.Y, SrcAffine[3], SrcAffine[5]);
        OGR_G_AddPoint(Ring, XPixel, YPixel, 0);
    }
    
    OGRGeometryH OutGeom = OGR_G_CreateGeometry(wkbPolygon);
    OGR_G_AddGeometry(OutGeom, Ring);
    return OutGeom;
}

static bool
RasterCut(char* DstRaster, char** SrcRasterList, int NumSrcRasters, v2* CutPolygon, int NumPoints)
{
    // Must have already called GDALAllRegister().
    
    char VSIName[64] = {0};
    GDALDatasetH SrcDS;
    if (NumSrcRasters > 1)
    {
        GDALBuildVRTOptionsForBinary* OptionsList = NULL;
        GDALBuildVRTOptions* Options = GDALBuildVRTOptionsNew(SrcRasterList, OptionsList);
        
        time_t Now = time(0);
        sprintf(VSIName, "/vsimem/%d.vrt", (int)Now);
        int Error = 0;
        SrcDS = GDALBuildVRT(VSIName, NumSrcRasters, NULL, SrcRasterList, Options, &Error);
        
        GDALBuildVRTOptionsFree(Options);
    }
    else
    {
        SrcDS = GDALOpen(SrcRasterList[0], GA_ReadOnly);
    }
    
    // Mosaico de entrada
    
    double SrcAffine[6] = {0};
    GDALGetGeoTransform(SrcDS, SrcAffine);
    double SrcMinX = SrcAffine[0];
    double SrcMaxX = SrcAffine[0] + (SrcAffine[1] * GDALGetRasterXSize(SrcDS));
    double SrcMinY = SrcAffine[3] + (SrcAffine[5] * GDALGetRasterYSize(SrcDS));
    double SrcMaxY = SrcAffine[3];
    
    const char* Proj = GDALGetProjectionRef(SrcDS);
    int NumBands = GDALGetRasterCount(SrcDS);
    int SrcXSize = GDALGetRasterXSize(SrcDS);
    int SrcYSize = GDALGetRasterYSize(SrcDS);
    
    // Poligono de corte
    int LeftPixel = INT_MAX, TopPixel = INT_MAX, RightPixel = INT_MIN, BottomPixel = INT_MIN;
    for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        v2 Point = CutPolygon[PointIdx];
        int XPixel = CoordToPixel(Point.X, SrcAffine[0], SrcAffine[1]);
        int YPixel = CoordToPixel(Point.Y, SrcAffine[3], SrcAffine[5]);
        
        LeftPixel   = (XPixel < LeftPixel)   ? XPixel : LeftPixel;
        RightPixel  = (XPixel > RightPixel)  ? XPixel : RightPixel;
        TopPixel    = (YPixel < TopPixel)    ? YPixel : TopPixel;
        BottomPixel = (YPixel > BottomPixel) ? YPixel : BottomPixel;
    }
    LeftPixel = Clamp(LeftPixel, 0, SrcXSize);
    RightPixel = Clamp(RightPixel, 0, SrcXSize);
    TopPixel = Clamp(TopPixel, 0, SrcYSize);
    BottomPixel = Clamp(BottomPixel, 0, SrcYSize);
    
    int DstXSize = RightPixel - LeftPixel;
    int DstYSize = BottomPixel - TopPixel;
    double MinX = SrcAffine[0] + (LeftPixel * SrcAffine[1]);
    double MaxY = SrcAffine[3] + (TopPixel * SrcAffine[5]);
    
    double DstAffine[6] = { MinX, SrcAffine[1], SrcAffine[2], MaxY, SrcAffine[4], SrcAffine[5] };
    OGRGeometryH PLGeom = XYGeomToPLGeom(CutPolygon, NumPoints, SrcAffine, SrcXSize, SrcYSize);
    
    // Imagem de saida
    GDALDriverH Driver = GDALGetDriverByName("GTiff");
    char** CreateOptions = NULL;
    CreateOptions = CSLSetNameValue(CreateOptions, "COMPRESS", "LZW");
    GDALDatasetH DstDS = GDALCreate(Driver, DstRaster, DstXSize, DstYSize, NumBands, GDALGetRasterDataType(SrcDS), CreateOptions);
    CSLDestroy(CreateOptions);
    
    GDALSetProjection(DstDS, Proj);
    GDALSetGeoTransform(DstDS, DstAffine);
    for (int BandIdx = 1; BandIdx <= NumBands; BandIdx++)
    {
        GDALRasterBandH InBand = GDALGetRasterBand(SrcDS, BandIdx);
        int HasNoData = NULL;
        double NoData = GDALGetRasterNoDataValue(InBand, &HasNoData);
        GDALRasterBandH OutBand = GDALGetRasterBand(DstDS, BandIdx);
        GDALSetRasterNoDataValue(OutBand, NoData);
    }
    
    // Processamento
    GDALWarpOptions* WarpOptions = GDALCreateWarpOptions();
    WarpOptions->hSrcDS = SrcDS;
    WarpOptions->hDstDS = DstDS;
    WarpOptions->nBandCount = NumBands;
    WarpOptions->panSrcBands = (int*)CPLMalloc(sizeof(int) * NumBands);
    WarpOptions->panDstBands = (int*)CPLMalloc(sizeof(int) * NumBands);
    for (int BandIdx = 1; BandIdx <= NumBands; BandIdx++)
    {
        WarpOptions->panSrcBands[BandIdx-1] = BandIdx;
        WarpOptions->panDstBands[BandIdx-1] = BandIdx;
    }
    WarpOptions->hCutline = PLGeom;
    WarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(SrcDS, 0, DstDS, 0, FALSE, 0, 1);
    WarpOptions->pfnTransformer = GDALGenImgProjTransform;
    
    GDALWarpOperation Warp;
    Warp.Initialize(WarpOptions);
    Warp.ChunkAndWarpImage(0, 0, GDALGetRasterXSize(DstDS), GDALGetRasterYSize(DstDS));
    
    GDALDestroyGenImgProjTransformer(WarpOptions->pTransformerArg);
    GDALDestroyWarpOptions(WarpOptions);
    GDALClose(DstDS);
    GDALClose(SrcDS);
    if (NumSrcRasters > 1) VSIUnlink(VSIName);
    
    return true;
}