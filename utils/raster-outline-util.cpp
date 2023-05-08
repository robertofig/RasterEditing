#include "raster-outline.h"
#include "geotypes-shp.h"

#define USAGE_CODE \
"Program for creating polygon outlines of raster images. If image does not have a 'no-data' value set, it returns its bounding box instead.\n" \
" Usage: raster-outline.exe [input] [--bbox]\n" \
"     > input: Path to raster from which to extract the outline.\n" \
"     > --bbox (optional): Flag that forces output to be bounding box instead of tight outline.\n" \
" Example 1: raster-outline.exe image.ecw\n" \
" Example 2: raster-outline.exe image.tif --bbox\n" \
" Output: Shapefile with a single multipolygon feature, created at the same path as image.\n"

#define BBOX_BUFFER_SIZE (sizeof(ring_info) + sizeof(v2) * 5)

int main(int Argc, char** Argv)
{
    if (Argc < 2 || Argc > 3)
    {
        fprintf(stderr, "Error: Incorrect number of parameters.\n" USAGE_CODE);
        return -1;
    }
    
    GDALAllRegister();
    LoadSystemInfo();
    
    GDALDatasetH DS = GDALOpen(Argv[1], GA_ReadOnly);
    if (!DS)
    {
        fprintf(stderr, "Error: Could not open raster file.\n");
        return -1;
    }
    
    u8 BBoxBuffer[BBOX_BUFFER_SIZE] = {0};
    poly_info Poly = {0};
    
    int RasterHasNoData = 0;
    GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
    double NoData = GDALGetRasterNoDataValue(Band, &RasterHasNoData);
    if (!memcmp(Argv[2], "--bbox", 6) || !RasterHasNoData)
    {
        // Outline is raster BBox.
        Poly.NumVertices = 5;
        Poly.NumRings = 1;
        
        int Width = GDALGetRasterXSize(DS);
        int Height = GDALGetRasterYSize(DS);
        double Affine[6];
        GDALGetGeoTransform(DS, Affine);
        
        usz PolyDataSize = BBOX_BUFFER_SIZE;
        Poly.Rings = (ring_info*)BBoxBuffer;
        Poly.Rings->NumVertices = 5;
        Poly.Rings->Vertices[0] = Poly.Rings->Vertices[4] = V2(Affine[0], Affine[3]);
        Poly.Rings->Vertices[1] = V2(Affine[0] + (Width * Affine[1]), Affine[3]);
        Poly.Rings->Vertices[2] = V2(Affine[0] + (Width * Affine[1]), Affine[3] + (Height * Affine[5]));
        Poly.Rings->Vertices[3] = V2(Affine[0], Affine[3] + (Height * Affine[5]));
    }
    else
    {
        int BandCount = GDALGetRasterCount(DS);
        Poly = RasterToOutline(DS, NoData, 0, TestType_EqualAll, BandCount, NULL);
    }
    
    GDALClose(DS);
    
    // Write rings to shapefile.
    
    usz OutFullSize = (sizeof(i32) * Poly.NumRings) + (sizeof(v2) * Poly.NumVertices) + 152 + 108 + 35;
    u8* OutShp = (u8*)GetMemory(OutFullSize, 0, MEM_READ|MEM_WRITE);
    u8* OutDbf = OutShp + OutFullSize - 35;
    u8* OutShx = OutDbf - 108;
    
    shapefile Out = CreatePolygon(OutShp, OutShx, OutDbf);
    shp_feature Feat = AddFeature(&Out, Poly.NumRings, Poly.NumVertices);
    
    for (ring_info* Ring = Poly.Rings; Ring; Ring = Ring->Next)
    {
        AddRing(&Feat, ShpRing_Outer, Ring->NumVertices, Ring->Vertices, 0, 0, sizeof(v2));
        for (ring_info* ChildRing = Ring->Child; ChildRing; ChildRing = ChildRing->Next)
        {
            AddRing(&Feat, ShpRing_Inner, ChildRing->NumVertices, ChildRing->Vertices, 0, 0, sizeof(v2));
        }
    }
    
    // Write shapefile to disk.
    
    string InPath = String(Argv[1], strlen(Argv[1]), 0, EC_ASCII);
    usz SepIdx = CharInString('\\', InPath, RETURN_IDX_AFTER|SEARCH_REVERSE);
    if (SepIdx != INVALID_IDX)
    {
        InPath.Base += SepIdx;
        InPath.WriteCur -= SepIdx;
    }
    
    char OutPathBuf[MAX_PATH_SIZE] = {0};
    path OutPath = Path(OutPathBuf);
    AppendStringToPath(InPath, &OutPath);
    
    usz InsertIdx = OutPath.WriteCur = CharInString('.', OutPath, RETURN_IDX_AFTER|SEARCH_REVERSE);
    AppendStringToString(StrLit("shp"), &OutPath);
    file Shp = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    OutPath.WriteCur = InsertIdx;
    AppendStringToString(StrLit("shx"), &OutPath);
    file Shx = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    OutPath.WriteCur = InsertIdx;
    AppendStringToString(StrLit("dbf"), &OutPath);
    file Dbf = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    WriteEntireFile(Shp, Buffer(Out.ShpFilePtr, Out.ShpFileSize, 0));
    WriteEntireFile(Shx, Buffer(Out.ShxFilePtr, Out.ShxFileSize, 0));
    WriteEntireFile(Dbf, Buffer(Out.DbfFilePtr, Out.DbfFileSize, 0));
    
    CloseFileHandle(Shp);
    CloseFileHandle(Shx);
    CloseFileHandle(Dbf);
    
    return 0;
}