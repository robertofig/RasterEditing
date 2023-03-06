#include "raster-outline.h"
#include "geotypes-shp.h"

int main(int Argc, char** Argv)
{
    if (Argc != 4)
    {
        fprintf(stderr, "Error: Incorrect number of parameters.\n");
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
    
    int BandsToCheck = atoi(Argv[3]);
    if (!memcmp(Argv[2], "nodata", 6))
    {
        GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
        int RasterHasNoData = 0;
        double Value = GDALGetRasterNoDataValue(Band, &RasterHasNoData);
        
        Poly = (RasterHasNoData) ? RasterToOutline(DS, Value, BandsToCheck, NULL) : BBoxOutline(DS, BBoxBuffer);
    }
    else if (!memcmp(Argv[2], "bbox", 4))
    {
        Poly = BBoxOutline(DS, BBoxBuffer);
    }
    else
    {
        double Value = atof(Argv[2]);
        Poly = RasterToOutline(DS, Value, BandsToCheck, NULL);
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