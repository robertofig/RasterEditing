#include "gdal_priv.h"
#include "gdal_utils.h"
#include "ogr_api.h"
#include "gdalwarper.h"

#include <stdio.h>
#include <time.h>

internal int
Clamp(int Target, int MinValue, int MaxValue)
{
    int Result = Min(Max(Target, MinValue), MaxValue);
    return Result;
}

internal int
CoordToPixel(double TargetCoord, double FirstCoord, double PixelSize)
{
    int Pixel = (int)((TargetCoord - FirstCoord) / PixelSize);
    return Pixel;
}

internal OGRGeometryH
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

struct re_mosaic
{
    char VSIName[64];
    GDALDatasetH DS;
    double Affine[6];
    double MinX, MaxX, MinY, MaxY;
    int NumBands;
    int XSize, YSize;
    const char* Proj;
};

internal re_mosaic
LoadRastersFromList(char** SrcRasterList, int NumSrcRasters)
{
    re_mosaic Src = {0};
    if (NumSrcRasters > 1)
    {
        GDALBuildVRTOptionsForBinary* OptionsList = NULL;
        GDALBuildVRTOptions* Options = GDALBuildVRTOptionsNew(SrcRasterList, OptionsList);
        
        time_t Now = time(0);
        sprintf(Src.VSIName, "/vsimem/%d.vrt", (int)Now);
        int Error = 0;
        Src.DS = GDALBuildVRT(Src.VSIName, NumSrcRasters, NULL, SrcRasterList, Options, &Error);
        
        GDALBuildVRTOptionsFree(Options);
    }
    else
    {
        Src.DS = GDALOpen(SrcRasterList[0], GA_ReadOnly);
    }
    
    GDALGetGeoTransform(Src.DS, Src.Affine);
    Src.MinX = Src.Affine[0];
    Src.MaxX = Src.Affine[0] + (Src.Affine[1] * GDALGetRasterXSize(Src.DS));
    Src.MinY = Src.Affine[3] + (Src.Affine[5] * GDALGetRasterYSize(Src.DS));
    Src.MaxY = Src.Affine[3];
    
    Src.Proj = GDALGetProjectionRef(Src.DS);
    Src.NumBands = GDALGetRasterCount(Src.DS);
    Src.XSize = GDALGetRasterXSize(Src.DS);
    Src.YSize = GDALGetRasterYSize(Src.DS);
    
    return Src;
}

external GDALDatasetH
RasterCut(char* DstRaster, char** SrcRasterList, int NumSrcRasters, v2* CutPolygon, int NumPoints)
{
    GDALDatasetH DstDS = 0;
    
    // Must have already called GDALAllRegister().
    
    re_mosaic Src = LoadRastersFromList(SrcRasterList, NumSrcRasters);
    if (!Src.DS)
    {
        return DstDS;
    }
    
    // Processes cut polygon
    
    int LeftPixel = INT_MAX, TopPixel = INT_MAX, RightPixel = INT_MIN, BottomPixel = INT_MIN;
    for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        v2 Point = CutPolygon[PointIdx];
        int XPixel = CoordToPixel(Point.X, Src.Affine[0], Src.Affine[1]);
        int YPixel = CoordToPixel(Point.Y, Src.Affine[3], Src.Affine[5]);
        
        LeftPixel   = (XPixel < LeftPixel)   ? XPixel : LeftPixel;
        RightPixel  = (XPixel > RightPixel)  ? XPixel : RightPixel;
        TopPixel    = (YPixel < TopPixel)    ? YPixel : TopPixel;
        BottomPixel = (YPixel > BottomPixel) ? YPixel : BottomPixel;
    }
    LeftPixel = Clamp(LeftPixel, 0, Src.XSize);
    RightPixel = Clamp(RightPixel, 0, Src.XSize);
    TopPixel = Clamp(TopPixel, 0, Src.YSize);
    BottomPixel = Clamp(BottomPixel, 0, Src.YSize);
    
    int DstXSize = RightPixel - LeftPixel;
    int DstYSize = BottomPixel - TopPixel;
    double MinX = Src.Affine[0] + (LeftPixel * Src.Affine[1]);
    double MaxY = Src.Affine[3] + (TopPixel * Src.Affine[5]);
    
    double DstAffine[6] = { MinX, Src.Affine[1], Src.Affine[2], MaxY, Src.Affine[4], Src.Affine[5] };
    OGRGeometryH PLGeom = XYGeomToPLGeom(CutPolygon, NumPoints, Src.Affine, Src.XSize, Src.YSize);
    
    // Creates output image
    
    GDALDriverH Driver = GDALGetDriverByName("GTiff");
    char** CreateOptions = NULL;
    CreateOptions = CSLSetNameValue(CreateOptions, "COMPRESS", "LZW");
    GDALRasterBandH Band = GDALGetRasterBand(Src.DS, 1);
    GDALDataType DType = GDALGetRasterDataType(Band);
    DstDS = GDALCreate(Driver, DstRaster, DstXSize, DstYSize, Src.NumBands, DType, CreateOptions);
    CSLDestroy(CreateOptions);
    
    GDALSetProjection(DstDS, Src.Proj);
    GDALSetGeoTransform(DstDS, DstAffine);
    for (int BandIdx = 1; BandIdx <= Src.NumBands; BandIdx++)
    {
        GDALRasterBandH InBand = GDALGetRasterBand(Src.DS, BandIdx);
        int HasNoData = NULL;
        double NoData = GDALGetRasterNoDataValue(InBand, &HasNoData);
        GDALRasterBandH OutBand = GDALGetRasterBand(DstDS, BandIdx);
        GDALSetRasterNoDataValue(OutBand, NoData);
    }
    
    // Cuts the mosaic
    
    GDALWarpOptions* WarpOptions = GDALCreateWarpOptions();
    WarpOptions->hSrcDS = Src.DS;
    WarpOptions->hDstDS = DstDS;
    WarpOptions->nBandCount = Src.NumBands;
    WarpOptions->panSrcBands = (int*)CPLMalloc(sizeof(int) * Src.NumBands);
    WarpOptions->panDstBands = (int*)CPLMalloc(sizeof(int) * Src.NumBands);
    for (int BandIdx = 1; BandIdx <= Src.NumBands; BandIdx++)
    {
        WarpOptions->panSrcBands[BandIdx-1] = BandIdx;
        WarpOptions->panDstBands[BandIdx-1] = BandIdx;
    }
    WarpOptions->hCutline = PLGeom;
    WarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(Src.DS, 0, DstDS, 0, FALSE, 0, 1);
    WarpOptions->pfnTransformer = GDALGenImgProjTransform;
    
    GDALWarpOperation Warp;
    Warp.Initialize(WarpOptions);
    Warp.ChunkAndWarpImage(0, 0, GDALGetRasterXSize(DstDS), GDALGetRasterYSize(DstDS));
    
    GDALDestroyGenImgProjTransformer(WarpOptions->pTransformerArg);
    GDALDestroyWarpOptions(WarpOptions);
    GDALClose(Src.DS);
    if (NumSrcRasters > 1) VSIUnlink(Src.VSIName);
    
    return DstDS;
}