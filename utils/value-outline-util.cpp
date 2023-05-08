#include "raster-outline.h"
#include "geotypes-shp.h"

#define USAGE_CODE \
"Program for creating polygon outlines around groups of pixels in a raster image, based on some classification rule regarding a target pixel value.\n" \
" Usage: value-outline.exe --input [src path] --value [target value] (options)\n" \
"    --input: Path to raster from which to extract the outline.\n" \
"    --rule: What rule to test which pixels to outline. Possible rules are:\n" \
"        a) equal: pixels equal to value (default).\n" \
"        b) not_equal: pixels different from value.\n" \
"        c) more_than: pixels equal to or higher than value.\n" \
"        d) less_than: pixels equal to or lower than value.\n" \
"        e) between: pixels equal to or between two values.\n" \
"    --value: Pixel value of the outline. A second value must also be passed\n" \
"            in case rule is 'between'. Ignored otherwise.\n" \
"    --bands: Bands to investigate the value in. Must be comma-separated, ex: 2,4,5,8.\n" \
"    --any: Any one band having the value will count for that pixel.\n" \
"    --all: All bands must have the value for that pixel to count (default).\n" \
" Example 1: value-outline.exe --input image.ecw --rule not_equal --value 255 --bands 1,2,3 --any\n" \
" Example 2: value-outline.exe --input image.tif --rule between --value 410.3 1128.4 --bands 1\n" \
" Output: Shapefile with a single multipolygon feature, created at the current working directory.\n"

enum cl_flags
{
    Flag_None,
    Flag_Input,
    Flag_Rule,
    Flag_Value,
    Flag_Bands
};

int main(int Argc, char** Argv)
{
    if (Argc == 1 || (Argc == 2 && !memcmp(Argv[1], "-help", 5)))
    {
        fprintf(stderr, USAGE_CODE);
        return 0;
    }
    
    // Parse arguments.
    
    char* SrcPath = NULL;
    char* Rule = NULL;
    double ValueA = -INFINITY;
    double ValueB = -INFINITY;
    char* Bands = NULL;
    bool AnyBand = false;
    
    cl_flags Flag = Flag_None;
    for (int ArgIdx = 1; ArgIdx < Argc; ArgIdx++)
    {
        char* Arg = Argv[ArgIdx];
        
        if (!memcmp(Arg, "--input", 7))      Flag = Flag_Input;
        else if (!memcmp(Arg, "--rule", 6))  Flag = Flag_Rule;
        else if (!memcmp(Arg, "--value", 7)) Flag = Flag_Value;
        else if (!memcmp(Arg, "--bands", 7)) Flag = Flag_Bands;
        else if (!memcmp(Arg, "--any", 5))
        {
            AnyBand = true;
        }
        else
        {
            if (Flag == Flag_Input)
            {
                if (!SrcPath) { SrcPath = Arg; }
                Flag = Flag_None;
            }
            else if (Flag == Flag_Rule)
            {
                if (!Rule) { Rule = Arg; }
                Flag = Flag_None;
            }
            else if (Flag == Flag_Value)
            {
                if (ValueA == -INFINITY)      { ValueA = atof(Arg); }
                else if (ValueB == -INFINITY) { ValueB = atof(Arg); }
            }
            else if (Flag == Flag_Bands)
            {
                if (!Bands) { Bands = Arg; }
                Flag = Flag_None;
            }
        }
    }
    
    // Prepare parameters and call function.
    
    if (!SrcPath)
    {
        fprintf(stderr, "Error: No source raster specified.\n");
        return -1;
    }
    
    if (ValueA == -INFINITY)
    {
        fprintf(stderr, "Error: Value is not specificed.\n");
        return -1;
    }
    
    GDALAllRegister();
    LoadSystemInfo();
    
    GDALDatasetH DS = GDALOpen(SrcPath, GA_ReadOnly);
    if (!DS)
    {
        fprintf(stderr, "Error: Could not open raster file.\n");
        return -1;
    }
    
    int BandCount = 0;
    int* BandIdx = NULL;
    if (Bands)
    {
        string BandList = String(Bands, strlen(Bands), 0, EC_ASCII);
        BandIdx = (int*)GetMemoryFromHeap((BandList.Size/2 + 1) * sizeof(int));
        for (usz Idx = 0
             ; (Idx = CharInString(',', BandList, RETURN_IDX_FIND)) != INVALID_IDX
             ; BandList.Base += Idx, BandList.WriteCur -= Idx)
        {
            string Number = String(BandList.Base, Idx, 0, EC_ASCII);
            BandIdx[BandCount++] = StringToInt(Number);
            Idx++;
        }
        BandIdx[BandCount++] = StringToInt(BandList);
    }
    if (BandCount == 0) { GDALGetRasterCount(DS); }
    
    test_type Test = TestType_EqualAll;
    if (!memcmp(Rule, "not_equal", 9))      { Test = TestType_NotEqualAll; }
    else if (!memcmp(Rule, "more_than", 9)) { Test = TestType_MoreThanAll; }
    else if (!memcmp(Rule, "less_than", 9)) { Test = TestType_LessThanAll; }
    else if (!memcmp(Rule, "between", 7))
    {
        if (ValueB == -INFINITY)
        {
            fprintf(stderr, "Error: Second value is not specificed.\n");
            return -1;
        }
        Test = TestType_BetweenAll;
    }
    if (AnyBand) { Test = (test_type)((int)Test + 1); }
    
    poly_info Poly = RasterToOutline(DS, ValueA, ValueB, Test, BandCount, BandIdx);
    
    timing Trace = {0};
    StartTiming(&Trace);
    GDALClose(DS);
    
    StopTiming(&Trace);
    fprintf(stdout, "Closing DS: %f\n", Trace.Diff);
    StartTiming(&Trace);
    
    // Write rings to shapefile.
    
    usz OutFullSize = (sizeof(i32) * Poly.NumRings) + (sizeof(v2) * Poly.NumVertices) + 152 + 108 + 35;
    u8* OutShp = (u8*)GetMemory(OutFullSize, 0, MEM_READ|MEM_WRITE);
    u8* OutDbf = OutShp + OutFullSize - 35;
    u8* OutShx = OutDbf - 108;
    
    shapefile Out = CreatePolygon(OutShp, OutShx, OutDbf);
    shp_feature Feat = AddFeature(&Out, Poly.NumRings, Poly.NumVertices);
    
    for (ring_info* Ring = Poly.Rings; Ring; Ring = Ring->Next)
    {
        AddRing(&Feat, ShpRing_Outer, Ring->NumVertices, Ring->Vertices, 0, 0, sizeof(v2));
        for (ring_info* ChildRing = Ring->Child; ChildRing; ChildRing = ChildRing->Next)
        {
            AddRing(&Feat, ShpRing_Inner, ChildRing->NumVertices, ChildRing->Vertices, 0, 0, sizeof(v2));
        }
    }
    
    // Write shapefile to disk.
    
    string InPath = String(SrcPath, strlen(SrcPath), 0, EC_ASCII);
    usz SepIdx = CharInString('\\', InPath, RETURN_IDX_AFTER|SEARCH_REVERSE);
    if (SepIdx != INVALID_IDX)
    {
        InPath.Base += SepIdx;
        InPath.WriteCur -= SepIdx;
    }
    
    char OutPathBuf[MAX_PATH_SIZE] = {0};
    path OutPath = Path(OutPathBuf);
    AppendStringToPath(InPath, &OutPath);
    
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
    
    StopTiming(&Trace);
    fprintf(stdout, "Writing shapefile: %f\n", Trace.Diff);
    
    return 0;
}