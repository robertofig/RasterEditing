#ifndef RASTER_OUTLINE_H
#define RASTER_OUTLINE_H

#include "gdal.h"
#include "geotypes-base.h"

struct ring_info
{
    ring_info* Next;
    
    u32 Type; // 0: Outer, 1: Inner
    u32 NumVertices;
    v2 Vertices[0];
};

struct poly_info
{
    u32 NumVertices;
    u32 NumRings;
    ring_info* Rings;
};

external poly_info RasterDSToOutline(GDALDatasetH DS);
external poly_info RasterPathToOutline(char* SrcRasterPath);
external void FreePolyInfo(poly_info Poly);


#if !defined(RASTER_EDITING_STATIC_LINKING)
#include "raster-outline.cpp"
#endif

#endif //RASTER_OUTLINE_H
