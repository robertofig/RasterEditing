#include "raster-outline.h"
#include "geotypes-shp.h"

#define USAGE_CODE "Usage: raster-outline.exe [input_raster] [value] [bands]\n" \
"    > input_raster: Path to raster from which to extract the outline.\n" \
"    > value: Pixel value of the outline. Alternatively 'nodata' to outline valid pixels, " \
"or 'bbox' to outline the border of the raster.\n" \
"    > bands: Bands to investigate the value in. Must be comma-separated, ex: 2,4,5,8.\n" \
"Example: raster-outline.exe path/to/raster/image.ecw 255 1,2,3\n" \
"Output: Shapefile with a single multipolygon feature, created at the same path as image.\n"

int main(int Argc, char** Argv)
{
    if (Argc != 4)
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
    
    string BandList = String(Argv[3], strlen(Argv[3]), 0, EC_ASCII);
    int* Bands = (int*)GetMemoryFromHeap((BandList.Size/2 + 1) * sizeof(int));
    int BandCount = 0;
    for (usz Idx = 0
         ; (Idx = CharInString(',', BandList, RETURN_IDX_FIND)) != INVALID_IDX
         ; BandList.Base += Idx, BandList.WriteCur -= Idx)
    {
        string Number = String(BandList.Base, Idx, 0, EC_ASCII);
        Bands[BandCount++] = StringToInt(Number);
        Idx++;
    }
    Bands[BandCount++] = StringToInt(BandList);
    
    if (!memcmp(Argv[2], "nodata", 6))
    {
        GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
        int RasterHasNoData = 0;
        double Value = GDALGetRasterNoDataValue(Band, &RasterHasNoData);
        
        Poly = (RasterHasNoData) ? RasterToOutline(DS, Value, 0, TestType_NotEqual, BandCount, Bands) : BBoxOutline(DS, BBoxBuffer);
    }
    else if (!memcmp(Argv[2], "bbox", 4))
    {
        Poly = BBoxOutline(DS, BBoxBuffer);
    }
    else
    {
        double Value = atof(Argv[2]);
        //Poly = RasterToOutline(DS, Value, 0, TestType_Equal, BandCount, Bands);
        Poly = RasterToOutline(DS, 1, 254, TestType_Between, BandCount, Bands);
    }
    
    GDALClose(DS);
    
    usz OutFullSize = (sizeof(i32) * Poly.NumRings) + (sizeof(v2) * Poly.NumVertices) + 152 + 108 + 35;
    u8* OutShp = (u8*)GetMemory(OutFullSize, 0, MEM_READ|MEM_WRITE);
    u8* OutDbf = OutShp + OutFullSize - 35;
    u8* OutShx = OutDbf - 108;
    
    shapefile Out = CreatePolygon(OutShp, OutShx, OutDbf);
    shp_feature Feat = AddFeature(&Out, Poly.NumRings, Poly.NumVertices);
    
    ring_info* Ring = Poly.Rings;
    for (u32 RingIdx = 0; RingIdx < Poly.NumRings; RingIdx++)
    {
        shp_ring_type Type = (Ring->Type == 0) ? ShpRing_Outer : ShpRing_Inner;
        AddRing(&Feat, Type, Ring->NumVertices, Ring->Vertices, 0, 0, sizeof(v2));
        Ring = Ring->Next;
    }
    
    char OutPathBuf[MAX_PATH_SIZE] = {0};
    path OutPath = Path(OutPathBuf);
    AppendStringToPath(String(Argv[1], strlen(Argv[1]), 0, EC_ASCII), &OutPath);
    
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