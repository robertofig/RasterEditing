#include "raster-outline.h"
#include "geotypes-shp.h"

#define USAGE_CODE \
"Usage: raster-outline.exe [input_raster] [operation] [value] [bands]\n" \
"    > input_raster: Path to raster from which to extract the outline.\n" \
"    > operation: The operation to be performed. Options: 'equal', 'not_equal', " \
"'bigger_than', 'bigger_or_equal_to', 'less_than', 'less_or_equal_to', " \
"'between'.\n" \
"    > value: Pixel value of the operation. If the operation is 'between', type " \
"two values comma-separated, ex: 406,1027.\n" \
"    > bands: Bands to investigate the value in. Must be comma-separated, ex: " \
"2,4,5,8.\n\n" \
"Example: raster-outline.exe path/to/image.ecw less_than 186 2,3,5\n" \
"Output: Shapefile with a single multipolygon feature, created at the same path as" \
"image, containing all pixels less than 186 in bands 2, 3 and 5.\n"

#define MemCmp(A, B) !memcmp(A, B, sizeof(B))

struct parsed_args
{
    GDALDatasetH DS;
    test_type Type;
    f64 ValueA;
    f64 ValueB;
    int* Bands;
    int BandCount;
};

internal bool
ParseArgs(char** Argv, parsed_args* Parse)
{
    GDALDatasetH DS = GDALOpen(Argv[1], GA_ReadOnly);
    if (!DS)
    {
        fprintf(stderr, "Error: Could not open raster file.\n");
        return false;
    }
    
    test_type Type;
    if (MemCmp(Argv[2], "equal")) Type = TestType_Equal;
    else if (MemCmp(Argv[2], "not_equal")) Type = TestType_NotEqual;
    else if (MemCmp(Argv[2], "bigger_than")) Type = TestType_BiggerThan;
    else if (MemCmp(Argv[2], "bigger_or_equal_to")) Type = TestType_BiggerOrEqualTo;
    else if (MemCmp(Argv[2], "less_than")) Type = TestType_LessThan;
    else if (MemCmp(Argv[2], "less_or_equal_to")) Type = TestType_LessOrEqualTo;
    else if (MemCmp(Argv[2], "between")) Type = TestType_Between;
    else
    {
        fprintf(stderr, "Error: Operation is not supported.\n");
        return false;
    }
    
    f64 ValueA = INF64, ValueB = INF64;
    if (Type == TestType_Between)
    {
        for (char* Ptr = Argv[3]; *Ptr; Ptr++)
        {
            if (Ptr[0] == ',')
            {
                ValueA = StringToFloat(String(Argv[3], Ptr - Argv[3], 0, EC_ASCII));
                ValueB = StringToFloat(StringC(Ptr+1, EC_ASCII));
                break;
            }
        }
    }
    else
    {
        ValueA = StringToFloat(StringC(Argv[3], EC_ASCII));
    }
    
    if (ValueA == INF64 || (Type == TestType_Between && ValueB == INF64))
    {
        fprintf(stderr, "Error: Values are not valid.\n");
        return false;
    }
    
    usz BandListSize = strlen(Argv[4]);
    buffer BandArrMem = GetMemoryFromHeap((BandListSize/2 + 1) * sizeof(int));
    if (!BandArrMem.Base)
    {
        fprintf(stderr, "Error: Not enough memory.\n");
        return false;
    }
    int* Bands = (int*)BandArrMem.Base;
    int BandCount = 0;
    for (char* Ptr = Argv[4], *Start = Argv[4]; ; Ptr++)
    {
        if (Ptr[0] == ',' || Ptr[0] == '\0')
        {
            string Number = String(Start, Ptr-Start, 0, EC_ASCII);
            isz BandNumber = StringToInt(Number);
            if (BandNumber == ISZ_MAX)
            {
                fprintf(stderr, "Error: Band number not valid.\n");
                return false;
            }
            Bands[BandCount++] = (int)BandNumber;
            
            if (Ptr[0] == '\0') break;
            else Start = Ptr+1;
        }
    }
    
    Parse->DS = DS;
    Parse->Type = Type;
    Parse->ValueA = ValueA;
    Parse->ValueB = ValueB;
    Parse->Bands = Bands;
    Parse->BandCount = BandCount;
    
    return true;
}

int main(int Argc, char** Argv)
{
    if (Argc != 5)
    {
        fprintf(stderr, "Error: Incorrect number of parameters.\n" USAGE_CODE);
        return -1;
    }
    
    GDALAllRegister();
    LoadSystemInfo();
    
    parsed_args Parse = {0};
    if (!ParseArgs(Argv, &Parse))
    {
        return -1;
    }
    
    poly_info Poly = RasterToOutline(Parse.DS, Parse.ValueA, Parse.ValueB, Parse.Type,
                                     Parse.BandCount, Parse.Bands);
    if (Poly.NumVertices == 0)
    {
        fprintf(stderr, "Error: RasterToOutline() failed.\n");
        return -1;
    }
    
    usz OutFullSize = ((sizeof(i32) * Poly.NumRings)
                       + (sizeof(v2) * Poly.NumVertices)
                       + 152 + 108 + 35); // Headers for SHP, SHX and DBF.
    buffer OutShapefile = GetMemory(OutFullSize, 0, MEM_WRITE);
    if (!OutShapefile.Base)
    {
        fprintf(stderr, "Error: Not enough memory.\n");
        return -1;
    }
    
    u8* OutShp = (u8*)OutShapefile.Base;
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
    AppendStringToString(StringLit("shp"), &OutPath);
    file Shp = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    OutPath.WriteCur = InsertIdx;
    AppendStringToString(StringLit("shx"), &OutPath);
    file Shx = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    OutPath.WriteCur = InsertIdx;
    AppendStringToString(StringLit("dbf"), &OutPath);
    file Dbf = CreateNewFile(OutPath.Base, WRITE_SOLO|FORCE_CREATE);
    
    WriteEntireFile(Shp, Buffer(Out.ShpFilePtr, Out.ShpFileSize, 0));
    WriteEntireFile(Shx, Buffer(Out.ShxFilePtr, Out.ShxFileSize, 0));
    WriteEntireFile(Dbf, Buffer(Out.DbfFilePtr, Out.DbfFileSize, 0));
    
    CloseFileHandle(Shp);
    CloseFileHandle(Shx);
    CloseFileHandle(Dbf);
    
    return 0;
}