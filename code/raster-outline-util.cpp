#include "raster-outline.cpp"
#include "geotypes-shp.h"

int main(int Argc, char** Argv)
{
    if (Argc > 1)
    {
        GDALAllRegister();
        LoadSystemInfo();
        
        poly_info Poly = RasterPathToOutline(Argv[1]);
        
        timing Trace;
        StartTiming(&Trace);
        
        void* OutShp = GetMemory(Megabyte(10), 0, MEM_READ|MEM_WRITE);
        void* OutShx = GetMemory(Megabyte(10), 0, MEM_READ|MEM_WRITE);
        void* OutDbf = GetMemory(Megabyte(10), 0, MEM_READ|MEM_WRITE);
        
        shapefile Out = CreatePolygon(OutShp, OutShx, OutDbf);
        shp_feature Feat = AddFeature(&Out, Poly.NumRings, Poly.NumVertices);
        
        ring_info* Ring = Poly.Rings;
        for (u32 RingIdx = 0; RingIdx < Poly.NumRings; RingIdx++)
        {
            shp_ring_type Type = (Ring->Type == 0) ? ShpRing_Outer : ShpRing_Inner;
            AddRing(&Feat, Type, Ring->NumVertices, Ring->Vertices, 0, 0, sizeof(v2));
            Ring = Ring->Next;
        }
        
        file Shp = CreateNewFile(L"Z:\\RasterEditing\\data\\Polygonize\\test.shp", WRITE_SOLO|FORCE_CREATE);
        file Shx = CreateNewFile(L"Z:\\RasterEditing\\data\\Polygonize\\test.shx", WRITE_SOLO|FORCE_CREATE);
        file Dbf = CreateNewFile(L"Z:\\RasterEditing\\data\\Polygonize\\test.dbf", WRITE_SOLO|FORCE_CREATE);
        
        WriteEntireFile(Shp, Buffer(Out.ShpFilePtr, Out.ShpFileSize, 0));
        WriteEntireFile(Shx, Buffer(Out.ShxFilePtr, Out.ShxFileSize, 0));
        WriteEntireFile(Dbf, Buffer(Out.DbfFilePtr, Out.DbfFileSize, 0));
        
        CloseFileHandle(Shp);
        CloseFileHandle(Shx);
        CloseFileHandle(Dbf);
        
        StopTiming(&Trace);
        fprintf(stdout, "Writing to shapefile: %f\n", Trace.Diff);
        
        FreePolyInfo(Poly);
    }
    
    return 0;
}