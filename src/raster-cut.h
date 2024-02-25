#ifndef RASTER_CUT_H
//=========================================================================
// raster-cut.h
//
// Module for mosaicking and cutting rasters into a single output raster.
// Given a list of rasters and a single polygon (without holes), the
// resulting image is all the pixels in the original image within the
// area of the polygon.
//=========================================================================
#define RASTER_CUT_H

#include "gdal.h"
#include "geotypes-base.h"

external GDALDatasetH RasterCut(char* DstRaster, char** SrcRasterList, int NumSrcRasters,
                                v2* CutPolygon, int NumPoints);

/* Creates output polygon given a [SrcRasterList] and a [CutPolygon]. The number
|  of rasters in the list and number of vertices in the polygon are passed in
|  [NumSrcRasters] and [NumPoints], respectively. [DstRaster] must be a path
 |  in the filesystem with the output filename. Currently only accepts output
|  files in TIFF format.
|--- Return: GDAL Dataset containing the created raster.*/


#if !defined(RASTER_EDITING_STATIC_LINKING)
#include "raster-cut.cpp"
#endif

#endif //RASTER_CUT_H
