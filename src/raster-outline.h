#ifndef RASTER_OUTLINE_H
//=========================================================================
// raster-outline.h
//
// Module for creating polygon outlines of raster values. Given a target
// value and a source raster, it creates polygons around groups of pixels
// found in the raster selected from some test rule passed by the user,
// e.g. all pixels that are equal to the target value.
//
// The function RasterToOutline() returns a poly_info struct holding the
// resulting polygons as a linked list of rings. These rings are ordered
// by outer ring -> inner rings, followed by other outer rings. The last
// vertex of the ring repeats the first one, to facilitate serialization.
//
// Ex: if the result is three polygons, A, B and C, with A having two
// inner rings, B none and C one inner ring, they layout in memory will be:
//   A-outer -> A-inner -> A-inner -> B-outer -> C-outer -> C-inner
//
// The [Rings] member of the returned poly_info points to the first ring,
// which is always an outer ring. The following ones are read by iterating
// through the [Next] member of ring_info, and checking the [Type] member
// to see if the ring is outer or inner. The read ends when [Next] returns
// a NULL pointer.
//
// Alternatively the BBoxOutline() function can be used to extract the
// polygon outline of the entire image area. Memory is not allocated by
// the internals, but instead expected to be passed by the application,
// of at least size BBOX_BUFFER_SIZE. The result can be read the same way
// as the poly_info of RasterToOutline().
//=========================================================================
#define RASTER_OUTLINE_H

#include "gdal.h"
#include "geotypes-base.h"

#define BBOX_BUFFER_SIZE (sizeof(ring_info) + sizeof(v2) * 5)

enum test_type
{
    TestType_Equal,      // Pixel == target value A.
    TestType_NotEqual,   // Pixel != target value A.
    TestType_BiggerThan, // Pixel >= target value A.
    TestType_LessThan,   // Pixel <= target value A.
    TestType_Between,    // target value A >= Pixel <= target value B.
    TestType_NotBetween  // Pixel < target value A OR Pixel > target value B.
};

struct ring_info
{
    ring_info* Next;
    
    u32 Type; // 0: Outer, 1: Inner
    u32 NumVertices; // Number of vertices in this ring alone.
    v2 Vertices[0];
};

struct poly_info
{
    u32 NumVertices; // Total number of vertices in all rings.
    u32 NumRings;    // Total number of rings (outer and inner).
    ring_info* Rings;
};

external poly_info RasterToOutline(GDALDatasetH DS, double ValueA, double ValueB,
                                   test_type TestType, int BandCount, int* BandIdx);

/* Creates outline in raster [DS] around pixels that match the [TestType]
 |  condition of a pixel value. For test types of Equal, NotEqual, BiggerThan,
|  and LessThan, the conditional value is passed in [ValueA], and [ValueB]
|  can be passed anything (won't be used); for test of type Between, the lower
|  value is passed in [ValueA], and the bigger in [ValueB].
|  
|  [BandIdx] is an array of band numbers to be used for testing, and must be
 |  at least [BandCount] big. Each pixel must pass the test on all specified
 |  bands to be eligible for outlining. [BandIdx] can also be NULL, in this
 |  case the function on [BandCount] number of bands starting from the first
 |  raster bands.
|--- Return: poly_info object with all the outlines, or empty if failure.*/

external poly_info BBoxOutline(GDALDatasetH DS, u8* BBoxBuffer);

/* Creates outline of image boundary of raster [DS] in memory [BBoxBuffer].
|  [BBoxBuffer] must be allocated by the caller, with BBOX_BUFFER_SIZE bytes
|  and writing permission (it can be just an array on the stack).
|--- Return: poly_info object with image boundary outline. */

external void FreePolyInfo(poly_info Poly);

/* Use after calling RasterToOutline() to free the memory allocated in
|  it. Can also be called after BBoxOutline() or in case RasterToOutline()
|  fails, in which case it will do nothing. */


#if !defined(RASTER_EDITING_STATIC_LINKING)
#include "raster-outline.cpp"
#endif

#endif //RASTER_OUTLINE_H
