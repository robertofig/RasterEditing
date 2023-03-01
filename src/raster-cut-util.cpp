#include "raster-cut.cpp"

#include "geotypes-shp.h"


int main(int Argc, char** Argv)
{
    if (Argc < 4)
    {
        fprintf(stderr, "Error: Insufficient args.\n");
        return -1;
    }
    
    char ShpPathBuf[MAX_PATH_SIZE] = {0};
    path ShpPath = Path(ShpPathBuf);
    AppendStringToPath(String(Argv[2], strlen(Argv[2]), 0, EC_UTF8), &ShpPath);
    
    shapefile Shape = OpenAndImportShp(ShpPathBuf);
    if (Shape.NumFeatures == 1
        && (Shape.Type == ShpType_Polygon || Shape.Type == ShpType_PolygonM || Shape.Type == ShpType_PolygonZM))
    {
        shp_feature Feat = GetFeature(&Shape, 0);
        if (Feat.NumParts == 1)
        {
            GDALAllRegister();
            
            shp_part Geom = GetGeometry(Feat, 0);
            RasterCut(Argv[1], Argv+3, Argc-3, Geom.XY, Geom.NumPoints);
        }
    }
    
    return 0;
}