#ifndef GEOTYPES_SHP_H
//=========================================================================
// geotypes-shp.h
//
// Module for reading and writing shapefiles. Writing a new feature
// requires full knowledge of the number of parts and points of its
// geometry, which must have been created and edited in full prior to
// beginning of serialization. Features must be written to entirely
// before a new feature can be added.
//
// Reading:
//   1. Load entire content of SHP, SHX and DBF files to memory.
//   2. Call ImportShp() to create parsed shapefile object.
//   3. Iterate through NumFeatures with GetFeature().
//   4. Read feature data with GetGeometry(), GetFieldByIdx/Name().
//
// Writing:
//   1. Allocate 3 memory buffers, with enough space to fit the entire
//      SHP, SHX and DBF files respectively.
//   2. Call one of the Create...() functions.
//   3. Call CreateField() on the returned shapefile object for each
//      field to be created.
//   4. Create a new shp_feature object with AddFeature().
//   5. Add the required geometries on the feature, and data for each
//      field created above (if not, geom/field is NULL).
//   6. Repeat from step 4 for each feature in the shapefile.
//   7. After finished adding features, save buffers to disk.
//=========================================================================
#define GEOTYPES_SHP_H

#include "tinybase-types.h"
#include "tinybase-memory.h"
#include "tinybase-strings.h"
#include "tinybase-platform.h"

#include "geotypes-base.h"

//=================================
// Shapefile enums
//=================================

// The (...)ZM geoms are called (...)Z in the specs, but are in fact XYZM points.
// Changed to (...)ZM here for clarity.

enum shp_type : i32
{
    ShpType_Null         = 0,
    ShpType_Point        = 1,
    ShpType_Polyline     = 3,
    ShpType_Polygon      = 5,
    ShpType_MultiPoint   = 8,
    ShpType_PointZM      = 11,
    ShpType_PolylineZM   = 13,
    ShpType_PolygonZM    = 15,
    ShpType_MultiPointZM = 18,
    ShpType_PointM       = 21,
    ShpType_PolylineM    = 23,
    ShpType_PolygonM     = 25,
    ShpType_MultiPointM  = 28,
    ShpType_MultiPatch   = 31,
};

enum shp_ring_type
{
    ShpRing_Outer,
    ShpRing_Inner
};

enum shp_patch_type : i32
{
    ShpMultiPatch_TriangleStrip = 0,
    ShpMultiPatch_TriangleFan   = 1,
    ShpMultiPatch_OuterRing     = 2,
    ShpMultiPatch_InnerRing     = 3,
    ShpMultiPatch_FirstRing     = 4,
    ShpMultiPatch_Ring          = 5,
    ShpMultiPatch_None          = 6  // Not part of the specs. 
};

enum shp_field_type
{
    ShpField_String,
    ShpField_Integer,
    ShpField_Real,
    ShpField_Boolean,
    ShpField_Date
};

//=================================
// Shapefile structs
//=================================

struct shapefile
{
    u8* ShpFilePtr;
    u8* ShxFilePtr;
    u8* DbfFilePtr;
    i32 ShxFileSize;
    i32 ShpFileSize;
    i32 DbfFileSize;
    
    shp_type Type;
    i32 NumFeatures;
    i32 NumFields;
    i32 LastFeatIdx;
    
    bool InternalAlloc;
};

struct shp_feature
{
    shapefile* Shape;
    i32 NumParts;
    i32 NumPoints;
    u8* ShpRecord;
    u8* ShxRecord;
    u8* DbfRecord;
};

struct shp_part
{
    i32 NumPoints;
    v2* XY;
    f64* Z;
    f64* M;
    shp_patch_type PatchType;
};

struct shp_field
{
    shp_field_type Type;
    char* Name;
    usz NameSize;
    bool IsDeleted;
    
    char* String;
    usz StringSize;
    isz Integer;
    f64 Real;
    bool Boolean;
    struct { u16 Year, Month, Day; } Date;
};

//=================================
// Creation functions
//=================================

external shapefile CreatePoint(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePointM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePointZM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreateMultiPoint(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreateMultiPointM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreateMultiPointZM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolyline(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolylineM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolylineZM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolygon(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolygonM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreatePolygonZM(void* ShpPtr, void* ShxPtr, void* DbfPtr);
external shapefile CreateMultiPatch(void* ShpPtr, void* ShxPtr, void* DbfPtr);

external bool CreateField(shapefile* Shape, char* FieldName, u8 FieldLen, u8 Precision, shp_field_type Type);

/* Creates a new field descriptor for the shapefile. [FieldName] must be zero-terminated
 |  and 11 bytes or fewer in length. [FieldLen] is how many bytes to reserve for data type
|  String, or how many digits to reserve for Integer and Real (Boolean and Date are not
|  affected). [Precision] is how many of the digits in data type Real to reserve for the
|  decimal places (must be less than FieldLen, unused for other types).
|--- Return: true if successful, false if not. */


//=================================
// Read data functions
//=================================

external shapefile ImportShp(buffer Shp, buffer Shx, buffer Dbf);

/* Imports shapefile from previously opened [.shp], [.shx] and [.dbf] files. User is
|  responsible for opening and reading those files into the memory buffers, and later
 |  freeing up that memory. The buffers must not be touched until the user finishes
|  using the shapefile, as the library reads directly from them.
|--- Return: parsed shapefile object if successful, empty object if not. */

external shapefile OpenAndImportShp(void* ShpFilePath);

/* Opens and imports shapefile given the path for the [.shp] file. Unlike ImportShp(),
|  memory for it is handled by the library, and user must call CloseShp() later to
 |  free up resources. ShpFilePath must be in the OS unicode encoding, and the
 |  corresponding [.shx] and [.dbf] files must also be present in the same folder.
 |--- Return: parsed shapefile object if successful, empty object if not. */

external void CloseShp(shapefile* Shape);

/* Frees memory from shapefile object (if it was created with OpenAndImportShp()), and
|  zeroes out its memory.
 |--- Return: nothing. */

external shp_feature GetFeature(shapefile* Shape, i32 TargetIdx);

/* Gets the Nth feature from Shape. TargetIdx goes from 0 to (Shape->NumFeatures - 1).
|--- Return: shp_feature object. */

external shp_part GetGeometry(shp_feature Feat, i32 TargetIdx);

/* Gets the Nth geometry part from Feat. TargetIdx goes from 0 to (Feat.NumParts - 1).
|--- Return: shp_part object. */

external shp_field GetFieldByIdx(shp_feature Feat, i32 TargetIdx);

/* Gets the Nth field from Feat. TargetIdx goes from 0 to (Feat.NumFields - 1).
|--- Return: shp_field object. */

external shp_field GetFieldByName(shp_feature Feat, char* TargetName);

/* Gets the field from Feat by its name. TargetName must be a zero-terminated ASCII string.
|--- Return: shp_field object if found, empty object if name not found. */


//=================================
// Write data functions
//=================================

external shp_feature AddFeature(shapefile* Shape, i32 NumParts, i32 NumPoints);

/* Creates a new feature in [Shape]. Entire feature must be written to before calling
 |  this function again. [NumParts] is the number of parts in a geometry (number of rings
|  in a polygon, number of geoms in a multitype geom, number of patches in a multipatch).
|  [NumPoints] is the total number of points in all parts.
|--- Example: A multipolygon geom with three polygons, each with a hole in them, will
|    amount to 6 parts. */

external bool AddPoint(shp_feature* Feat, v2 Vertex, f64 Z, f64 M);

/* Adds a new point for ShpType_Point(ZM) geometries. Call only once.
 |--- Return: true if successful, false otherwise. */

external bool AddPoints(shp_feature* Feat, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset);

/* Adds [NumPoints] points for ShpType_MultiPoint(ZM) geometries. [Vertex] is a pointer
|  to the first XY coords, likewise for [Z] and [M] (ignored if geom type does not have
|  them). [Offset] gives the offset in bytes between two points, and applies to [Vertex],
 |  [Z] and [M] alike.
|--- Return: true if successful, false otherwise. */

external bool AddLinestring(shp_feature* Feat, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset);

/* Adds a linestring of [NumPoints] vertices for ShpType_Polyline(ZM) geometries. Call
 |  this function for each linestring in the feature. The working of [Vertex], [Z], [M]
|  and [Offset] are the same as in AddPoints().
|--- Return: true if successful, false otherwise. */

external bool AddRing(shp_feature* Feat, shp_ring_type Type, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset);

/* Adds a ring ot [NumPoints] vertices for ShpType_Polygon(ZM) geometries. Call this
|  function for each ring in case the polygon has holes, differentiating outer ring
 |  from inner hole with [Type]. Call this function for each ring of each polygon in
|  the feature. The working of [Vertex], [Z], [M] and [Offset] are the same as in
 |  AddPoints().
|--- Return: true if successful, false otherwise. */

external bool AddPatch(shp_feature* Feat, shp_patch_type Type, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset);

/* Adds a patch of [NumPoints] vertices for ShpType_Multipatch geometries. Call this
|  function for each patch in the feature. [Type] determines the type of the patch.
 |  The working of [Vertex], [Z], [M] and [Offset] are the same as in AddPoints().
|--- Return: true if successful, false otherwise. */

external bool AddAttrString(shp_feature* Feat, char* String, usz StringSize, i32 FieldIdx);
external bool AddAttrInteger(shp_feature* Feat, isz Integer, i32 FieldIdx);
external bool AddAttrReal(shp_feature* Feat, f64 Real, i32 FieldIdx);
external bool AddAttrBoolean(shp_feature* Feat, bool Boolean, i32 FieldIdx);
external bool AddAttrDate(shp_feature* Feat, usz Year, usz Month, usz Day, i32 FieldIdx);

/* Adds data do a field of a feature [Feat]. The field pointed to by [FieldIdx] must
 |  have been previously created during layer setup. The field must be of the same
|  type as the version of AddAttr(...) used, else the function fails.
|--- Return: true if successful, false otherwise. */


#if !defined(GEOTYPES_STATIC_LINKING)
#include "geotypes-shp.cpp"
#endif

#endif //GEOTYPES_SHP_H
