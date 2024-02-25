#include "raster-cut.h"
#include "geotypes-shp.h"

#define USAGE_CODE \
"Usage: raster-cut.exe [output_raster] [cut_polygon] [input_raster ...]\n" \
"    > output_raster: Path where raster will be created (must be TIFF).\n" \
"    > cut_polygon: Path to polygon used for boundary (must be SHAPEFILE).\n" \
"    > input_raster: List of rasters used for input, separated by a space.\n" \
"Example: raster-cut.exe path/to/output.tif path/to/cut-poly.shp " \
"img1.tif img2.tif img3.tif\n" \
"Output: GeoTIFF raster with all input files mosaicked and cut to the polygon's " \
"border.\n"

int main(int Argc, char** Argv)
{
    if (Argc < 4)
    {
        fprintf(stderr, "Error: Insufficient number of parameters.\n" USAGE_CODE);
        return -1;
    }
    
    char ShpPathBuf[MAX_PATH_SIZE] = {0};
    path ShpPath = Path(ShpPathBuf);
    AppendStringToPath(StringC(Argv[2], EC_UTF8), &ShpPath);
    
    shapefile Shape = OpenAndImportShp(ShpPathBuf);
    if (Shape.Type == ShpType_Polygon
        || Shape.Type == ShpType_PolygonM
        || Shape.Type == ShpType_PolygonZM)
    {
        shp_feature Feat = GetFeature(&Shape, 0);
        if (Feat.NumParts == 1)
        {
            GDALAllRegister();
            
            shp_part Geom = GetGeometry(Feat, 0);
            GDALDatasetH DstDS = RasterCut(Argv[1], Argv+3, Argc-3, Geom.XY, Geom.NumPoints);
            GDALClose(DstDS);
        }
        else
        {
            fprintf(stderr, "Error: Polygon must not contain any holes.\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Error: Geometry was not of type Polygon.\n");
        return -1;
    }
    
    return 0;
}