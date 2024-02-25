#define SHAPEFILE_HEADER_SIZE 100

//=================================
// Internal structs
//=================================

#pragma pack(push, 1)

// SHP

struct shp_bbox2
{
    f64 XMin, YMin;
    f64 XMax, YMax;
};

struct shp_bbox4
{
    f64 XMin, YMin;
    f64 XMax, YMax;
    f64 ZMin, ZMax;
    f64 MMin, MMax;
};

struct shp_header
{
    // Big-Endian.
    i32 FileCode;       // OBS: Must be 9994.
    i32 Unused[5];
    i32 FileLength;     // OBS: Measured in 16-bit words.
    
    // Little-Endian.
    i32 Version;        // OBS: Must be 1000.
    shp_type Type;
    shp_bbox4 BBox;
};

struct shp_record
{
    // Big-Endian.
    i32 RecordNumber;   // OBS: First value is 1. 
    i32 ContentLength;  // OBS: Measured in 16-bit words.
};

// OBS: All structs below are Little-Endian.

struct shp_null
{
    shp_type Type;
};

struct shp_point
{
    shp_type Type;
    v2 Point;
};

struct shp_point_m
{
    shp_type Type;
    v3 Point;
};

struct shp_point_zm
{
    shp_type Type;
    v4 Point;
};

// OBS: The shp_z_block struct is present in every geom with a Z coord (except for shp_point_zm).
struct shp_z_block
{
    f64 ZMin;
    f64 ZMax;
    //f64 ZArray[NumPoints];
};

// OBS: The shp_m_block struct is present in every geom with a M coord (except for shp_point_m and shp_point_zm).
struct shp_m_block
{
    f64 MMin;
    f64 MMax;
    //f64 MArray[NumPoints];
};

struct shp_multipoint
{
    shp_type Type;
    shp_bbox2 BBox;
    i32 NumPoints;
    //v2 Points[NumPoints];
    //shp_z_block ZBlock;     OBS: Present in MultiPointZM.
    //shp_m_block MBlock;     OBS: Present in MultiPointM and MultiPointZM.
};

struct shp_multipart
{
    shp_type Type;
    shp_bbox2 BBox;
    i32 NumParts;
    i32 NumPoints;
    //i32 Parts[NumParts];    OBS: Array of idx of first point for each part.
    //shp_patch_type PartsType[NumParts];   OBS: Ordered array of part types, present in MultiPatch.
    //v2 Points[NumPoints];
    //shp_z_block ZBlock;     OBS: Present in PolylineZM, PolygonZM, and MultiPatch.
    //shp_m_block MBlock;     OBS: Present in PolylineM, PolylineZM, PolygonM, PolygonZM, and MultiPatch.
};

// SHX

typedef shp_header shx_header; // OBS: Both files have identical headers.

struct shx_record
{
    // Big-Endian.
    i32 DataOffset;
    i32 ContentLength;
};

// DBF

struct dbf_header
{
    u8 Version;
    u8 LastUpdate[3]; // OBS: YYYYMMDD, with year since 1900.
    i32 NumRecords;
    i16 HeaderSize;
    i16 RecordSize;   // OBS: Includes first verification byte.
    u8 _Reserved[20];
    // dbf_fd FDArray;   OBS: Variable size, depending on number of fields.
    // u8 EndOfHeader;   OBS: Must be 0xd.
};

enum dbf_field : u8
{
    DbfField_Char = 'C',
    DbfField_Date = 'D',
    DbfField_Float = 'F',
    DbfField_Logical = 'L',
    DbfField_Memo = 'M',
    DbfField_Numeric = 'N'
};

struct dbf_fd
{
    char Name[11];
    dbf_field Type;
    u8 _Reserved[4];
    u8 FieldLen;
    u8 Decimals;
    u8 _Reserved2[2];
    u8 WorkAreaID;
    u8 _Reserved3[10];
    u8 MDXFlag;
};

#pragma pack(pop)

//=================================
// Support functions
//=================================

internal void
_AddInBE(i32* CurrentValue, i32 AmountToAdd)
{
    i32 Value = FlipEndian32(*CurrentValue) + AmountToAdd;
    *CurrentValue = FlipEndian32(Value);
}

internal shp_ring_type
_GetRingOrientation(v2 A, v2 B, v2 C)
{
    v2 AB = B - A;
    v2 AC = C - A;
    f64 ZMag = Cross(AB, AC);
    return ZMag > 0 ? ShpRing_Outer : ShpRing_Inner;
}

internal void
_ReverseRing(v2* Points, i32 NumPoints)
{
    for (i32 Idx = 0; Idx < NumPoints/2; Idx++)
    {
        v2 Temp = Points[Idx];
        Points[Idx] = Points[NumPoints-Idx-1];
        Points[NumPoints-Idx-1] = Temp;
    }
}

internal void
_ReverseRing(f64* Points, i32 NumPoints)
{
    for (i32 Idx = 0; Idx < NumPoints/2; Idx++)
    {
        f64 Temp = Points[Idx];
        Points[Idx] = Points[NumPoints-Idx-1];
        Points[NumPoints-Idx-1] = Temp;
    }
}

internal shp_bbox2
_ShpBBox2(f64 XMin, f64 YMin, f64 XMax, f64 YMax)
{
    shp_bbox2 Result = { XMin, YMin, XMax, YMax };
    return Result;
}

#define SHP_BBOX2_DEFAULT _ShpBBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX)

//=================================
// Creation functions
//=================================

internal shapefile
_InitShapefile(void* ShpPtr, void* ShxPtr, void* DbfPtr, shp_type Type)
{
    shapefile Result = {0};
    Result.Type = Type;
    
    Result.ShpFilePtr = (u8*)ShpPtr;
    {
        shp_header* ShpHeader = (shp_header*)ShpPtr;
        ShpHeader->FileCode = 0xa270000; // 9994 in BE
        ShpHeader->FileLength = 0x32000000; // 50 in BE (number of 16-bit words for header).
        ShpHeader->Version = 1000;
        ShpHeader->Type = Type;
        
        f64 ZMin, ZMax, MMin, MMax;
        switch (Type)
        {
            case ShpType_PointZM:
            case ShpType_MultiPointZM:
            case ShpType_PolylineZM:
            case ShpType_PolygonZM:
            case ShpType_MultiPatch:
            {
                ZMin = DBL_MAX; ZMax = -DBL_MAX; MMin = DBL_MAX; MMax = -DBL_MAX;
            } break;
            
            case ShpType_PointM:
            case ShpType_MultiPointM:
            case ShpType_PolylineM:
            case ShpType_PolygonM:
            {
                ZMin = 0; ZMax = 0; MMin = DBL_MAX; MMax = -DBL_MAX;
            } break;
            
            default:
            {
                ZMin = 0; ZMax = 0; MMin = 0; MMax = 0;
            }
        }
        ShpHeader->BBox = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX, ZMin, ZMax, MMin, MMax };
    }
    Result.ShpFileSize = SHAPEFILE_HEADER_SIZE;
    
    Result.ShxFilePtr = (u8*)ShxPtr;
    CopyData(ShxPtr, SHAPEFILE_HEADER_SIZE, ShpPtr, SHAPEFILE_HEADER_SIZE);
    Result.ShxFileSize = Result.ShpFileSize;
    
    Result.DbfFilePtr = (u8*)DbfPtr;
    {
        dbf_header* DbfHeader = (dbf_header*)DbfPtr;
        DbfHeader->Version = 3;
        
        datetime Now = CurrentLocalTime();
        DbfHeader->LastUpdate[0] = Now.Year - 1900;
        DbfHeader->LastUpdate[1] = Now.Month;
        DbfHeader->LastUpdate[2] = Now.Day;
        DbfHeader->HeaderSize = sizeof(dbf_header) + 1; // OBS: This extra 1 is the last byte (0xd).
        DbfHeader->RecordSize = 1; // OBS: Verification byte.
        
        u8* EndOfHeader = (u8*)&DbfHeader[1];
        EndOfHeader[0] = 0xd;
        EndOfHeader[1] = 0x1a;
    }
    Result.DbfFileSize = sizeof(dbf_header) + 2;
    
    return Result;
}

external shapefile
CreatePoint(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_Point);
    return Result;
}

external shapefile
CreatePointM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PointM);
    return Result;
}

external shapefile
CreatePointZM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PointZM);
    return Result;
}

external shapefile
CreateMultiPoint(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_MultiPoint);
    return Result;
}

external shapefile
CreateMultiPointM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_MultiPointM);
    return Result;
}

external shapefile
CreateMultiPointZM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_MultiPointZM);
    return Result;
}

external shapefile
CreatePolyline(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_Polyline);
    return Result;
}

external shapefile
CreatePolylineM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PolylineM);
    return Result;
}

external shapefile
CreatePolylineZM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PolylineZM);
    return Result;
}

external shapefile
CreatePolygon(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_Polygon);
    return Result;
}

external shapefile
CreatePolygonM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PolygonM);
    return Result;
}

external shapefile
CreatePolygonZM(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_PolygonZM);
    return Result;
}

external shapefile
CreateMultiPatch(void* ShpPtr, void* ShxPtr, void* DbfPtr)
{
    shapefile Result = _InitShapefile(ShpPtr, ShxPtr, DbfPtr, ShpType_MultiPatch);
    return Result;
}

external bool
CreateField(shapefile* Shape, char* FieldName, u8 FieldLen, u8 Precision, shp_field_type Type)
{
    dbf_header* Dbf = (dbf_header*)Shape->DbfFilePtr;
    dbf_fd* FD = (dbf_fd*)(Shape->DbfFilePtr + sizeof(dbf_header)) + Shape->NumFields;
    
    usz FieldNameSize = strlen(FieldName);
    if (CopyData(FD->Name, 11, FieldName, FieldNameSize))
    {
        switch (Type)
        {
            case ShpField_String:
            {
                FD->Type = DbfField_Char;
                FD->FieldLen = FieldLen;
            } break;
            
            case ShpField_Integer:
            {
                FD->Type = DbfField_Numeric;
                FD->FieldLen = FieldLen;
            } break;
            
            case ShpField_Real:
            {
                FD->Type = DbfField_Float;
                FD->FieldLen = FieldLen;
                FD->Decimals = Precision;
            } break;
            
            case ShpField_Boolean:
            {
                FD->Type = DbfField_Logical;
                FD->FieldLen = 1;
            } break;
            
            case ShpField_Date:
            {
                FD->Type = DbfField_Date;
                FD->FieldLen = 8;
            } break;
        }
        
        u8* EndOfHeader = (u8*)&FD[1];
        EndOfHeader[0] = 0xd;
        EndOfHeader[1] = 0x1a;
        
        Dbf->HeaderSize += sizeof(dbf_fd);
        Dbf->RecordSize += FD->FieldLen;
        Shape->NumFields++;
        Shape->DbfFileSize += sizeof(dbf_fd);
        
        return true;
    }
    return false;
}

//=================================
// Read data functions
//=================================

external shapefile
ImportShp(buffer _Shp, buffer _Shx, buffer _Dbf)
{
    shapefile Result = {0};
    
    shp_header* Shp = (shp_header*)_Shp.Base;
    shx_header* Shx = (shx_header*)_Shx.Base;
    dbf_header* Dbf = (dbf_header*)_Dbf.Base;
    
    i32 ShpReadSize = 2 * FlipEndian32(Shp->FileLength);
    i32 ShxReadSize = 2 * FlipEndian32(Shx->FileLength);
    i32 DbfReadSize = Dbf->HeaderSize + (Dbf->RecordSize * Dbf->NumRecords);
    if (ShpReadSize > _Shp.WriteCur || ShxReadSize > _Shx.WriteCur || DbfReadSize > _Dbf.WriteCur
        || ShpReadSize < 0 || ShxReadSize < 0 || DbfReadSize < 0)
    {
        return Result;
    }
    
    i32 ShpNumRecords = 0;
    i32 LastFeatIdx = 0;
    u8* RecordPtr = (u8*)Shp + SHAPEFILE_HEADER_SIZE;
    u8* EndOfFile = (u8*)Shp + ShpReadSize;
    while (RecordPtr < EndOfFile)
    {
        ShpNumRecords++;
        LastFeatIdx = ((shp_record*)RecordPtr)->RecordNumber;
        i32 ContentLength = 2 * FlipEndian32(((shp_record*)RecordPtr)->ContentLength);
        RecordPtr += ContentLength + sizeof(shp_record);
    }
    LastFeatIdx = FlipEndian32(LastFeatIdx);
    
    i32 ShxNumRecords = ((i32)ShxReadSize - sizeof(shx_header)) / sizeof(shx_record);
    i32 DbfNumRecords = Dbf->NumRecords;
    if (ShpNumRecords != ShxNumRecords || ShpNumRecords != DbfNumRecords)
    {
        return Result;
    }
    
    Result.ShpFilePtr = (u8*)Shp;
    Result.ShpFileSize = ShpReadSize;
    Result.ShxFilePtr = (u8*)Shx;
    Result.ShxFileSize = ShxReadSize;
    Result.DbfFilePtr = (u8*)Dbf;
    Result.DbfFileSize = DbfReadSize;
    
    Result.Type = Shp->Type;
    Result.NumFeatures = ShpNumRecords;
    Result.NumFields = (Dbf->HeaderSize - sizeof(dbf_header) - 1) / sizeof(dbf_fd);
    Result.LastFeatIdx = LastFeatIdx;
    
    return Result;
}

external shapefile
OpenAndImportShp(void* ShpPathStr)
{
    shapefile Result = {0};
    
    path ShpPath = Path(ShpPathStr);
    ShpPath.WriteCur = StringLen(ShpPath, LEN_CSTRING);
    if (ShpPath.WriteCur > 0)
    {
        file ShpFile = OpenFileHandle(ShpPathStr, READ_SHARE);
        usz ShpFileSize = FileSizeOf(ShpFile);
        
        char AccompanyFiles[MAX_PATH_SIZE] = {0};
        path AccompanyPath = Path(AccompanyFiles);
        AppendDataToPath(ShpPath.Base, ShpPath.WriteCur, &AccompanyPath);
        usz ExtIdx = CharInString('.', AccompanyPath, RETURN_IDX_AFTER|SEARCH_REVERSE);
        
        AccompanyPath.WriteCur = ExtIdx;
        AppendStringToString(StringLit("shx"), &AccompanyPath);
        file ShxFile = OpenFileHandle(AccompanyFiles, READ_SHARE);
        usz ShxFileSize = FileSizeOf(ShxFile);
        
        AccompanyPath.WriteCur = ExtIdx;
        AppendStringToString(StringLit("dbf"), &AccompanyPath);
        file DbfFile = OpenFileHandle(AccompanyFiles, READ_SHARE);
        usz DbfFileSize = FileSizeOf(DbfFile);
        
        buffer Mem = {0};
        if (ShpFileSize > 0 && ShxFileSize > 0 && DbfFileSize > 0
            && (Mem = GetMemory(ShpFileSize + ShxFileSize + DbfFileSize, 0, MEM_READ|MEM_WRITE)).Base)
        {
            buffer Shp = Buffer(Mem.Base, 0, ShpFileSize);
            buffer Shx = Buffer(Shp.Base+ShpFileSize, 0, ShxFileSize);
            buffer Dbf = Buffer(Shx.Base+ShxFileSize, 0, DbfFileSize);
            
            if (ReadFromFile(ShpFile, &Shp, ShpFileSize, 0)
                && ReadFromFile(ShxFile, &Shx, ShxFileSize, 0)
                && ReadFromFile(DbfFile, &Dbf, DbfFileSize, 0))
            {
                Result = ImportShp(Shp, Shx, Dbf);
                Result.InternalAlloc = 1;
            }
        }
        
        CloseFileHandle(ShpFile);
        CloseFileHandle(ShxFile);
        CloseFileHandle(DbfFile);
    }
    
    return Result;
}

external void
CloseShp(shapefile* Shape)
{
    if (Shape->InternalAlloc)
    {
        usz MemSize = Shape->ShpFileSize + Shape->ShxFileSize + Shape->DbfFileSize;
        buffer Mem = Buffer(Shape->ShpFilePtr, 0, MemSize);
        FreeMemory(&Mem);
    }
    buffer ShapeBuffer = Buffer(Shape, sizeof(shapefile), sizeof(shapefile));
    ClearMemory(&ShapeBuffer);
}

internal shp_feature
_ReadRecordInfo(u8* Record)
{
    shp_feature Result = {0};
    shp_null* Content = (shp_null*)&((shp_record*)Record)[1];
    switch (Content->Type)
    {
        case ShpType_Null:
        {
            Result.NumParts = 0;
            Result.NumPoints = 0;
        } break;
        
        case ShpType_Point:
        case ShpType_PointM:
        case ShpType_PointZM:
        {
            Result.NumParts = 1;
            Result.NumPoints = 1;
        } break;
        
        case ShpType_MultiPoint:
        case ShpType_MultiPointM:
        case ShpType_MultiPointZM:
        {
            Result.NumParts = 1;
            Result.NumPoints = ((shp_multipoint*)Content)->NumPoints;
        } break;
        
        default:
        {
            Result.NumParts = ((shp_multipart*)Content)->NumParts;
            Result.NumPoints = ((shp_multipart*)Content)->NumPoints;
        }
    }
    Result.ShpRecord = Record;
    return Result;
}

external shp_feature
GetFeature(shapefile* Shape, i32 TargetIdx)
{
    shp_feature Result = {0};
    
    if (TargetIdx < Shape->NumFeatures)
    {
        shx_record* ShxRecords = (shx_record*)(Shape->ShxFilePtr + SHAPEFILE_HEADER_SIZE);
        i32 DataOffset = 2 * FlipEndian32(ShxRecords[TargetIdx].DataOffset);
        
        u8* ShpRecordPtr = Shape->ShpFilePtr + DataOffset;
        Result = _ReadRecordInfo(ShpRecordPtr);
        Result.Shape = Shape;
        Result.ShxRecord = (u8*)&ShxRecords[TargetIdx];
        
        dbf_header* DbfHeader = (dbf_header*)Shape->DbfFilePtr;
        Result.DbfRecord = Shape->DbfFilePtr + DbfHeader->HeaderSize + (DbfHeader->RecordSize * TargetIdx);
    }
    
    return Result;
}

external shp_part
GetGeometry(shp_feature Feat, i32 TargetIdx)
{
    shp_part Result = {0};
    
    if (TargetIdx < Feat.NumParts)
    {
        Result.PatchType = ShpMultiPatch_None;
        shp_null* Content = (shp_null*)(Feat.ShpRecord + sizeof(shp_record));
        switch (Content->Type)
        {
            case ShpType_Null: break;
            
            case ShpType_PointZM:
            case ShpType_PointM:
            case ShpType_Point:
            {
                Result.NumPoints = 1;
                Result.XY = &((shp_point*)Content)->Point;
                if (Content->Type == ShpType_PointZM)
                {
                    Result.Z = &((shp_point_zm*)Content)->Point.Z;
                    Result.M = &((shp_point_zm*)Content)->Point.M;
                }
                else if (Content->Type == ShpType_PointM)
                {
                    Result.M = &((shp_point_m*)Content)->Point.M;
                }
            } break;
            
            case ShpType_MultiPointZM:
            case ShpType_MultiPointM:
            case ShpType_MultiPoint:
            {
                Result.XY = (v2*)&((shp_multipoint*)Content)[1];
                u8* Ptr = (u8*)(Result.XY + Feat.NumPoints);
                switch (Content->Type)
                {
                    case ShpType_MultiPointZM:
                    {
                        Result.Z = (f64*)&((shp_z_block*)Ptr)[1];
                        Ptr = (u8*)(Result.Z + Feat.NumPoints);
                    }
                    case ShpType_MultiPointM:
                    {
                        Result.M = (f64*)&((shp_m_block*)Ptr)[1];
                    }
                }
                Result.NumPoints = Feat.NumPoints;
            } break;
            
            default:
            {
                i32* PartsIdx = (i32*)&((shp_multipart*)Content)[1];
                i32 GeomStartIdx = PartsIdx[TargetIdx];
                i32 GeomEndIdx = (TargetIdx+1 == Feat.NumParts) ? Feat.NumPoints : PartsIdx[TargetIdx+1];
                
                u8* Points = (u8*)(PartsIdx + Feat.NumParts);
                if (Content->Type == ShpType_MultiPatch)
                {
                    Result.PatchType = ((shp_patch_type*)Points)[TargetIdx];
                    Points += (sizeof(i32) * Feat.NumParts);
                }
                Result.XY = (v2*)Points + GeomStartIdx;
                
                Points = (u8*)((v2*)Points + Feat.NumPoints);
                switch (Content->Type)
                {
                    case ShpType_PolylineZM:
                    case ShpType_PolygonZM:
                    case ShpType_MultiPatch:
                    {
                        f64* ZArray = (f64*)&((shp_z_block*)Points)[1];
                        Result.Z = ZArray + GeomStartIdx;
                        Points = (u8*)(ZArray + Feat.NumPoints);
                    }
                    case ShpType_PolylineM:
                    case ShpType_PolygonM:
                    {
                        f64* MArray = (f64*)&((shp_m_block*)Points)[1];
                        Result.M = MArray + GeomStartIdx;
                    }
                }
                Result.NumPoints = GeomEndIdx - GeomStartIdx;
            } break;
        }
    }
    
    return Result;
}

internal shp_field
_ReadFieldInfo(dbf_fd* FD, usz FieldOffset, u8* DbfRecord)
{
    shp_field Result = {};
    
    Result.Name = FD->Name;
    for (char* Ptr = FD->Name; *Ptr && Ptr < &FD->Name[11]; Ptr++)
    {
        Result.NameSize++;
    }
    
    u8* Data = DbfRecord + FieldOffset + 1;
    u8* DataEnd = Data + FD->FieldLen;
    switch (FD->Type)
    {
        case DbfField_Char:
        {
            Result.Type = ShpField_String;
            Result.String = (char*)Data;
            for (u8* Ptr = Data; Ptr[0] != 0x20 && Ptr < DataEnd; Ptr++)
            {
                Result.StringSize++;
            }
        } break;
        
        case DbfField_Date:
        {
            Result.Type = ShpField_Date;
            Result.Date.Year = (u16)StringToUInt(String(Data, 4, 0, EC_ASCII));
            Result.Date.Month = (u16)StringToUInt(String(Data+4, 2, 0, EC_ASCII));
            Result.Date.Day = (u16)StringToUInt(String(Data+6, 2, 0, EC_ASCII));
        } break;
        
        case DbfField_Float:
        {
            Result.Type = ShpField_Real;
            while (*Data++ == 0x20) {}
            Result.Real = StringToFloat(String(Data, DataEnd-Data, 0, EC_ASCII));
        } break;
        
        case DbfField_Logical:
        {
            Result.Type = ShpField_Boolean;
            Result.Boolean = (*Data == 'Y' || *Data == 'y' || *Data == 'T' || *Data == 't') ? true : false;
        } break;
        
        case DbfField_Numeric:
        {
            while (*Data == 0x20) { Data++; }
            if (FD->Decimals > 0)
            {
                Result.Type = ShpField_Real;
                Result.Real = StringToFloat(String(Data, DataEnd-Data, 0, EC_ASCII));
            }
            else
            {
                Result.Type = ShpField_Integer;
                Result.Integer = StringToInt(String(Data, DataEnd-Data, 0, EC_ASCII));
            }
        } break;
    }
    Result.IsDeleted = (DbfRecord[0] == 0x20) ? false : true;
    
    return Result;
}

struct target_fd
{
    dbf_fd* FD;
    usz Offset;
};

internal target_fd
_GetFDByIdx(u8* Dbf, i32 TargetIdx)
{
    dbf_fd* FD = (dbf_fd*)(Dbf + sizeof(dbf_header));
    usz FieldOffset = 0;
    for (usz Count = 0; Count < TargetIdx; Count++)
    {
        FieldOffset += FD->FieldLen;
        FD++;
    }
    target_fd Result = { FD, FieldOffset };
    return Result;
}

external shp_field
GetFieldByIdx(shp_feature Feat, i32 TargetIdx)
{
    shp_field Result = {};
    if (TargetIdx < Feat.Shape->NumFields)
    {
        target_fd Field = _GetFDByIdx(Feat.Shape->DbfFilePtr, TargetIdx);
        Result = _ReadFieldInfo(Field.FD, Field.Offset, Feat.DbfRecord);
    }
    return Result;
}

external shp_field
GetFieldByName(shp_feature Feat, char* TargetName)
{
    shp_field Result = {};
    
    dbf_header* Header = (dbf_header*)Feat.Shape->DbfFilePtr;
    dbf_fd* FD = (dbf_fd*)(Feat.Shape->DbfFilePtr + sizeof(dbf_header));
    dbf_fd* FDEnd = (dbf_fd*)(Feat.Shape->DbfFilePtr + Header->HeaderSize);
    
    usz TargetNameSize = sizeof(TargetName);
    usz FieldOffset = 0;
    while (FD < FDEnd)
    {
        FieldOffset += FD->FieldLen;
        
        buffer Field = Buffer(FD->Name, 11, 0);
        buffer Target = Buffer(TargetName, TargetNameSize, 0);
        if (EqualBuffers(Field, Target))
        {
            Result = _ReadFieldInfo(FD, FieldOffset, Feat.DbfRecord);
            break;
        }
        FD++;
    }
    
    return Result;
}


//=================================
// Write data functions
//=================================

internal void
_SetDbfRecordToNull(u8* RecordPtr, dbf_header* Header, usz NumFields)
{
    // TODO: Write function.
    // - Numeric and Float types get set to ***
    // - Char type gets set to 0x20
    // - Date type gets set to 00000000
    // - Logical type (?)
    
    string Record = String(RecordPtr+1, 0, Header->RecordSize, EC_ASCII);
    dbf_fd* FD = (dbf_fd*)&Header[1];
    for (usz Count = 0; Count < NumFields; Count++)
    {
        mb_char SetChar;
        switch (FD->Type)
        {
            case DbfField_Date: SetChar = '0'; break;
            case DbfField_Float:
            case DbfField_Numeric: SetChar = '*'; break;
            default: SetChar = ' '; break;
        }
        AppendCharToStringNTimes(SetChar, FD->FieldLen, &Record);
        FD++;
    }
    RecordPtr[0] = 0x20;
}

external shp_feature
AddFeature(shapefile* Shape, i32 NumParts, i32 NumPoints)
{
    Shape->LastFeatIdx++;
    Shape->NumFeatures++;
    
    shp_record* NextShp = (shp_record*)(Shape->ShpFilePtr + Shape->ShpFileSize);
    NextShp->RecordNumber = FlipEndian32(Shape->LastFeatIdx);
    
    shx_record* NextShx = (shx_record*)(Shape->ShxFilePtr + Shape->ShxFileSize);
    NextShx->DataOffset = FlipEndian32(Shape->ShpFileSize / 2);
    shx_header* ShxHeader = (shx_header*)Shape->ShxFilePtr;
    
    Shape->ShpFileSize += sizeof(shp_record) + sizeof(shp_null);
    Shape->ShxFileSize += sizeof(shx_record);
    ShxHeader->FileLength = FlipEndian32(Shape->ShxFileSize / 2);
    
    u8* NextDbf = Shape->DbfFilePtr + Shape->DbfFileSize - 1;
    dbf_header* DbfHeader = (dbf_header*)Shape->DbfFilePtr;
    DbfHeader->NumRecords++;
    _SetDbfRecordToNull(NextDbf, DbfHeader, Shape->NumFields);
    Shape->DbfFileSize += DbfHeader->RecordSize;
    u8* EndOfRecord = NextDbf + DbfHeader->RecordSize;
    EndOfRecord[0] = 0x1a;
    
    shp_feature Result = { Shape, NumParts, NumPoints, (u8*)NextShp, (u8*)NextShx, NextDbf };
    return Result;
}

external bool
AddPoint(shp_feature* Feat, v2 Vertex, f64 Z, f64 M = DBL_MIN)
{
    shp_header* Header = (shp_header*)Feat->Shape->ShpFilePtr;
    if (Header->Type != ShpType_Point
        && Header->Type != ShpType_PointM
        && Header->Type != ShpType_PointZM)
    {
        return false;
    }
    
    shp_record* ShpRecord = (shp_record*)Feat->ShpRecord;
    shp_null* Content = (shp_null*)&ShpRecord[1];
    void* EndOfFile = NULL;
    
    switch (Header->Type)
    {
        case ShpType_Point:
        {
            shp_point* Point = (shp_point*)Content;
            Point->Type = Header->Type;
            Point->Point = Vertex;
            EndOfFile = Point + 1;
            
            Header->BBox.MMin = 0;
            Header->BBox.MMax = 0;
            Header->BBox.ZMin = 0;
            Header->BBox.ZMax = 0;
        } break;
        
        case ShpType_PointM:
        {
            shp_point_m* Point = (shp_point_m*)Content;
            Point->Type = Header->Type;
            Point->Point.XY = Vertex;
            Point->Point.M = M;
            EndOfFile = Point + 1;
            
            Header->BBox.MMin = Min(Header->BBox.MMin, M);
            Header->BBox.MMax = Max(Header->BBox.MMax, M);
            Header->BBox.ZMin = 0;
            Header->BBox.ZMax = 0;
        } break;
        
        case ShpType_PointZM:
        {
            shp_point_zm* Point = (shp_point_zm*)Content;
            Point->Type = Header->Type;
            Point->Point.XY = Vertex;
            Point->Point.M = M;
            Point->Point.Z = Z;
            EndOfFile = Point + 1;
            
            Header->BBox.MMin = Min(Header->BBox.MMin, M);
            Header->BBox.MMax = Max(Header->BBox.MMax, M);
            Header->BBox.ZMin = Min(Header->BBox.ZMin, Z);
            Header->BBox.ZMax = Max(Header->BBox.ZMax, Z);
        } break;
    }
    
    usz FileSize = (usz)EndOfFile - (usz)Header;
    usz FileLength = FileSize / 2;
    if (FileLength > I32_MAX)
    {
        return false;
    }
    
    Feat->Shape->ShpFileSize = (i32)FileSize;
    isz ContentLength = ((isz)EndOfFile - (isz)Content) / 2;
    ShpRecord->ContentLength = FlipEndian32(ContentLength);
    Header->FileLength = FlipEndian32(FileLength);
    
    shx_record* ShxRecord = (shx_record*)Feat->ShxRecord;
    ShxRecord->ContentLength = ShpRecord->ContentLength;
    
    Header->BBox.XMin = Min(Header->BBox.XMin, Vertex.X);
    Header->BBox.YMin = Min(Header->BBox.YMin, Vertex.Y);
    Header->BBox.XMax = Max(Header->BBox.XMax, Vertex.X);
    Header->BBox.YMax = Max(Header->BBox.YMax, Vertex.Y);
    
    shx_header* ShxHeader = (shx_header*)Feat->Shape->ShxFilePtr;
    ShxHeader->BBox = Header->BBox;
    
    return true;
}

internal void*
_AddZValues(shp_z_block* ZBlock, i32 PointCount, i32 NumPoints, f64* Z, i32 ZOffset)
{
    f64 ZMin = DBL_MAX, ZMax = -DBL_MAX;
    f64* WritePtr = (f64*)&ZBlock[1] + PointCount;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        ZMin = Min(ZMin, *Z);
        ZMax = Max(ZMax, *Z);
        *WritePtr++ = *Z;
        Z = (f64*)((u8*)Z + ZOffset);
    }
    ZBlock->ZMin = (PointCount > 0) ? Min(ZBlock->ZMin, ZMin) : ZMin;
    ZBlock->ZMax = (PointCount > 0) ? Max(ZBlock->ZMax, ZMax) : ZMax;
    
    return WritePtr;
}

internal void*
_AddMValues(shp_m_block* MBlock, i32 PointCount, i32 NumPoints, f64* M, i32 MOffset)
{
    f64 MMin = DBL_MAX, MMax = -DBL_MAX;
    f64* WritePtr = (f64*)&MBlock[1] + PointCount;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        MMin = Min(MMin, *M);
        MMax = Max(MMax, *M);
        *WritePtr++ = *M;
        M = (f64*)((u8*)M + MOffset);
    }
    MBlock->MMin = (PointCount > 0) ? Min(MBlock->MMin, MMin) : MMin;
    MBlock->MMax = (PointCount > 0) ? Max(MBlock->MMax, MMax) : MMax;
    
    return WritePtr;
}

external bool
AddPoints(shp_feature* Feat, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset)
{
    shp_header* Header = (shp_header*)Feat->Shape->ShpFilePtr;
    if (Header->Type != ShpType_MultiPoint
        && Header->Type != ShpType_MultiPointM
        && Header->Type != ShpType_MultiPointZM)
    {
        return false;
    }
    
    shp_record* ShpRecord = (shp_record*)Feat->ShpRecord;
    shp_multipoint* MultiPoint = (shp_multipoint*)&ShpRecord[1];
    v2* Points = (v2*)&MultiPoint[1];
    v2* WritePtr = Points + MultiPoint->NumPoints;
    
    shp_bbox2 BBox = (MultiPoint->NumPoints > 0) ? MultiPoint->BBox : SHP_BBOX2_DEFAULT;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        BBox.XMin = Min(BBox.XMin, Vertex->X);
        BBox.YMin = Min(BBox.YMin, Vertex->Y);
        BBox.XMax = Max(BBox.XMax, Vertex->X);
        BBox.YMax = Max(BBox.YMax, Vertex->Y);
        *WritePtr++ = *Vertex;
        Vertex = (v2*)((u8*)Vertex + Offset);
    }
    void* EndOfFile = WritePtr;
    
    u8* MBlockPtr = (u8*)Points + (sizeof(v2) * Feat->NumPoints);
    switch (Header->Type)
    {
        case ShpType_MultiPointZM:
        {
            shp_z_block* ZBlock = (shp_z_block*)MBlockPtr;
            EndOfFile = _AddZValues(ZBlock, MultiPoint->NumPoints, NumPoints, Z, Offset);
            Header->BBox.ZMin = Min(Header->BBox.ZMin, ZBlock->ZMin);
            Header->BBox.ZMax = Max(Header->BBox.ZMax, ZBlock->ZMax);
            MBlockPtr += sizeof(shp_z_block) + (sizeof(f64) * Feat->NumPoints);
        }
        case ShpType_MultiPointM:
        {
            shp_m_block* MBlock = (shp_m_block*)MBlockPtr;
            if (M != NULL)
            {
                EndOfFile = _AddMValues(MBlock, MultiPoint->NumPoints, NumPoints, M, Offset);
                Header->BBox.MMin = Min(Header->BBox.MMin, MBlock->MMin);
                Header->BBox.MMax = Max(Header->BBox.MMax, MBlock->MMax);
            }
            else
            {
                f64 NoData = DBL_MIN;
                EndOfFile = _AddMValues(MBlock, MultiPoint->NumPoints, NumPoints, &NoData, 0);
            }
        }
    }
    
    usz FileSize = (usz)EndOfFile - (usz)Header;
    isz FileLength = FileSize / 2;
    if (FileLength > I32_MAX)
    {
        return false;
    }
    
    Feat->Shape->ShpFileSize = (i32)FileSize;
    isz ContentLength = ((isz)EndOfFile - (isz)MultiPoint) / 2;
    ShpRecord->ContentLength = FlipEndian32(ContentLength);
    Header->FileLength = FlipEndian32(FileLength);
    
    shx_record* ShxRecord = (shx_record*)Feat->ShxRecord;
    ShxRecord->ContentLength = ShpRecord->ContentLength;
    
    MultiPoint->Type = Header->Type;
    MultiPoint->BBox = BBox;
    MultiPoint->NumPoints += NumPoints;
    
    Header->BBox.XMin = Min(Header->BBox.XMin, BBox.XMin);
    Header->BBox.YMin = Min(Header->BBox.YMin, BBox.YMin);
    Header->BBox.XMax = Max(Header->BBox.XMax, BBox.XMax);
    Header->BBox.YMax = Max(Header->BBox.YMax, BBox.YMax);
    
    shx_header* ShxHeader = (shx_header*)Feat->Shape->ShxFilePtr;
    ShxHeader->BBox = Header->BBox;
    
    return true;
}

external bool
AddLinestring(shp_feature* Feat, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset)
{
    shp_header* Header = (shp_header*)Feat->Shape->ShpFilePtr;
    if (Header->Type != ShpType_Polyline
        && Header->Type != ShpType_PolylineM
        && Header->Type != ShpType_PolylineZM)
    {
        return false;
    }
    
    shp_record* ShpRecord = (shp_record*)Feat->ShpRecord;
    shp_multipart* Polyline = (shp_multipart*)&ShpRecord[1];
    i32 PartIdx = Polyline->NumParts++;
    if (PartIdx >= Feat->NumParts)
    {
        return false;
    }
    
    i32* Parts = (i32*)&Polyline[1];
    Parts[PartIdx] = Polyline->NumPoints;
    
    v2* Points = (v2*)(Parts + Feat->NumParts);
    v2* WritePtr = Points + Polyline->NumPoints;
    
    shp_bbox2 BBox = (Polyline->NumPoints > 0) ? Polyline->BBox : SHP_BBOX2_DEFAULT;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        BBox.XMin = Min(BBox.XMin, Vertex->X);
        BBox.YMin = Min(BBox.YMin, Vertex->Y);
        BBox.XMax = Max(BBox.XMax, Vertex->X);
        BBox.YMax = Max(BBox.YMax, Vertex->Y);
        *WritePtr++ = *Vertex;
        Vertex = (v2*)((u8*)Vertex + Offset);
    }
    void* EndOfFile = WritePtr;
    
    u8* MBlockPtr = (u8*)Points + (sizeof(v2) * Feat->NumPoints);
    switch (Header->Type)
    {
        case ShpType_PolylineZM:
        {
            shp_z_block* ZBlock = (shp_z_block*)MBlockPtr;
            EndOfFile = _AddZValues(ZBlock, Polyline->NumPoints, NumPoints, Z, Offset);
            Header->BBox.ZMin = Min(Header->BBox.ZMin, ZBlock->ZMin);
            Header->BBox.ZMax = Max(Header->BBox.ZMax, ZBlock->ZMax);
            MBlockPtr += sizeof(shp_z_block) + (sizeof(f64) * Feat->NumPoints);
        }
        
        case ShpType_PolylineM:
        {
            shp_m_block* MBlock = (shp_m_block*)MBlockPtr;
            if (M != NULL)
            {
                EndOfFile = _AddMValues(MBlock, Polyline->NumPoints, NumPoints, M, Offset);
                Header->BBox.MMin = Min(Header->BBox.MMin, MBlock->MMin);
                Header->BBox.MMax = Max(Header->BBox.MMax, MBlock->MMax);
            }
            else
            {
                f64 NoData = DBL_MIN;
                EndOfFile = _AddMValues(MBlock, Polyline->NumPoints, NumPoints, &NoData, 0);
            }
        }
    }
    
    usz FileSize = (usz)EndOfFile - (usz)Header;
    usz FileLength = FileSize / 2;
    if (FileLength > I32_MAX)
    {
        return false;
    }
    
    Feat->Shape->ShpFileSize = (i32)FileSize;
    isz ContentLength = ((isz)EndOfFile - (isz)Polyline) / 2;
    ShpRecord->ContentLength = FlipEndian32(ContentLength);
    Header->FileLength = FlipEndian32(FileLength);
    
    shx_record* ShxRecord = (shx_record*)Feat->ShxRecord;
    ShxRecord->ContentLength = ShpRecord->ContentLength;
    
    Polyline->Type = Header->Type;
    Polyline->BBox = BBox;
    Polyline->NumPoints += NumPoints;
    
    Header->BBox.XMin = Min(Header->BBox.XMin, BBox.XMin);
    Header->BBox.YMin = Min(Header->BBox.YMin, BBox.YMin);
    Header->BBox.XMax = Max(Header->BBox.XMax, BBox.XMax);
    Header->BBox.YMax = Max(Header->BBox.YMax, BBox.YMax);
    
    shx_header* ShxHeader = (shx_header*)Feat->Shape->ShxFilePtr;
    ShxHeader->BBox = Header->BBox;
    
    return true;
}

external bool
AddRing(shp_feature* Feat, shp_ring_type RingType, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset)
{
    shp_header* Header = (shp_header*)Feat->Shape->ShpFilePtr;
    if (Header->Type != ShpType_Polygon
        && Header->Type != ShpType_PolygonM
        && Header->Type != ShpType_PolygonZM)
    {
        return false;
    }
    
    shp_record* ShpRecord = (shp_record*)Feat->ShpRecord;
    shp_multipart* Polygon = (shp_multipart*)&ShpRecord[1];
    i32 PartIdx = Polygon->NumParts++;
    if (PartIdx >= Feat->NumParts)
    {
        return false;
    }
    
    i32* Parts = (i32*)&Polygon[1];
    Parts[PartIdx] = Polygon->NumPoints;
    
    v2* Points = (v2*)(Parts + Feat->NumParts);
    v2* WritePtr = Points + Polygon->NumPoints;
    v2* AnchorPoint = WritePtr;
    
    shp_bbox2 BBox = (Polygon->NumPoints > 0) ? Polygon->BBox : SHP_BBOX2_DEFAULT;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        *WritePtr = *Vertex;
        
        BBox.XMin = Min(BBox.XMin, Vertex->X);
        BBox.YMin = Min(BBox.YMin, Vertex->Y);
        BBox.XMax = Max(BBox.XMax, Vertex->X);
        BBox.YMax = Max(BBox.YMax, Vertex->Y);
        if (WritePtr->Y < AnchorPoint->Y
            || (WritePtr->Y == AnchorPoint->Y
                && WritePtr->X > AnchorPoint->X))
        {
            AnchorPoint = WritePtr;
        }
        WritePtr++;
        Vertex = (v2*)((u8*)Vertex + Offset);
    }
    void* EndOfFile = WritePtr;
    
    v2* PointAfter = (AnchorPoint == (WritePtr-1)) ? (Points + Polygon->NumPoints) : AnchorPoint+1;
    v2* PointBefore = (AnchorPoint == (Points + Polygon->NumPoints)) ? (WritePtr-1) : AnchorPoint-1;
    shp_ring_type Orientation = _GetRingOrientation(*AnchorPoint, *PointBefore, *PointAfter);
    
    if (RingType != Orientation)
    {
        _ReverseRing(Points + Polygon->NumPoints, NumPoints);
    }
    
    u8* MBlockPtr = (u8*)Points + (sizeof(v2) * Feat->NumPoints);
    u8* ZBlockPtr = MBlockPtr;
    switch (Header->Type)
    {
        case ShpType_PolygonZM:
        {
            shp_z_block* ZBlock = (shp_z_block*)ZBlockPtr;
            EndOfFile = _AddZValues(ZBlock, Polygon->NumPoints, NumPoints, Z, Offset);
            Header->BBox.ZMin = Min(Header->BBox.ZMin, ZBlock->ZMin);
            Header->BBox.ZMax = Max(Header->BBox.ZMax, ZBlock->ZMax);
            MBlockPtr += sizeof(shp_z_block) + (sizeof(f64) * Feat->NumPoints);
            
            if (RingType != Orientation)
            {
                _ReverseRing((f64*)&ZBlock[1] + Polygon->NumPoints, NumPoints);
            }
        }
        
        case ShpType_PolygonM:
        {
            shp_m_block* MBlock = (shp_m_block*)MBlockPtr;
            if (M != NULL)
            {
                EndOfFile = _AddMValues(MBlock, Polygon->NumPoints, NumPoints, M, Offset);
                Header->BBox.MMin = Min(Header->BBox.MMin, MBlock->MMin);
                Header->BBox.MMax = Max(Header->BBox.MMax, MBlock->MMax);
            }
            else
            {
                f64 NoData = DBL_MIN;
                EndOfFile = _AddMValues(MBlock, Polygon->NumPoints, NumPoints, &NoData, 0);
            }
            
            if (RingType != Orientation)
            {
                _ReverseRing((f64*)&MBlock[1] + Polygon->NumPoints, NumPoints);
            }
        }
    }
    
    usz FileSize = (usz)EndOfFile - (usz)Header;
    usz FileLength = FileSize / 2;
    if (FileLength > I32_MAX)
    {
        return false;
    }
    
    Feat->Shape->ShpFileSize = (i32)FileSize;
    isz ContentLength = ((isz)EndOfFile - (isz)Polygon) / 2;
    ShpRecord->ContentLength = FlipEndian32(ContentLength);
    Header->FileLength = FlipEndian32(FileLength);
    
    shx_record* ShxRecord = (shx_record*)Feat->ShxRecord;
    ShxRecord->ContentLength = ShpRecord->ContentLength;
    
    Polygon->Type = Header->Type;
    Polygon->BBox = BBox;
    Polygon->NumPoints += NumPoints;
    
    Header->BBox.XMin = Min(Header->BBox.XMin, BBox.XMin);
    Header->BBox.YMin = Min(Header->BBox.YMin, BBox.YMin);
    Header->BBox.XMax = Max(Header->BBox.XMax, BBox.XMax);
    Header->BBox.YMax = Max(Header->BBox.YMax, BBox.YMax);
    
    shx_header* ShxHeader = (shx_header*)Feat->Shape->ShxFilePtr;
    ShxHeader->BBox = Header->BBox;
    
    return true;
}

external bool
AddPatch(shp_feature* Feat, shp_patch_type Type, i32 NumPoints, v2* Vertex, f64* Z, f64* M, i32 Offset)
{
    shp_header* Header = (shp_header*)Feat->Shape->ShpFilePtr;
    if (Header->Type != ShpType_MultiPatch)
    {
        return false;
    }
    
    shp_record* ShpRecord = (shp_record*)Feat->ShpRecord;
    shp_multipart* Multipatch = (shp_multipart*)&ShpRecord[1];
    i32 PartIdx = Multipatch->NumParts++;
    if (PartIdx >= Multipatch->NumParts)
    {
        return false;
    }
    
    i32* Parts = (i32*)&Multipatch[1];
    Parts[PartIdx] = Multipatch->NumPoints;
    shp_patch_type* PatchType = (shp_patch_type*)(Parts + Feat->NumParts);
    PatchType[PartIdx] = Type;
    
    v2* Points = (v2*)(PatchType + Feat->NumParts);
    v2* WritePtr = Points + Multipatch->NumPoints;
    
    shp_bbox2 BBox = (Multipatch->NumPoints > 0) ? Multipatch->BBox : SHP_BBOX2_DEFAULT;
    for (i32 Count = 0; Count < NumPoints; Count++)
    {
        BBox.XMin = Min(BBox.XMin, Vertex->X);
        BBox.YMin = Min(BBox.YMin, Vertex->Y);
        BBox.XMax = Max(BBox.XMax, Vertex->X);
        BBox.YMax = Max(BBox.YMax, Vertex->Y);
        *WritePtr++ = *Vertex;
        Vertex = (v2*)((u8*)Vertex + Offset);
    }
    void* EndOfFile = WritePtr;
    
    shp_z_block* ZBlock = (shp_z_block*)((u8*)Points + (sizeof(v2) * Feat->NumPoints));
    if (Z != NULL)
    {
        EndOfFile = _AddZValues(ZBlock, Multipatch->NumPoints, NumPoints, Z, Offset);
        Header->BBox.ZMin = Min(Header->BBox.ZMin, ZBlock->ZMin);
        Header->BBox.ZMax = Max(Header->BBox.ZMax, ZBlock->ZMax);
    }
    else
    {
        f64 NoData = 0;
        EndOfFile = _AddZValues(ZBlock, Multipatch->NumPoints, NumPoints, &NoData, 0);
    }
    
    shp_m_block* MBlock = (shp_m_block*)((u8*)ZBlock + sizeof(shp_z_block) + (sizeof(f64) * Feat->NumPoints));
    if (M != NULL)
    {
        EndOfFile = _AddMValues(MBlock, Multipatch->NumPoints, NumPoints, M, Offset);
        Header->BBox.MMin = Min(Header->BBox.MMin, MBlock->MMin);
        Header->BBox.MMax = Max(Header->BBox.MMax, MBlock->MMax);
    }
    else
    {
        f64 NoData = DBL_MIN;
        EndOfFile = _AddMValues(MBlock, Multipatch->NumPoints, NumPoints, &NoData, 0);
    }
    
    usz FileSize = (usz)EndOfFile - (usz)Header;
    usz FileLength = FileSize / 2;
    if (FileLength > I32_MAX)
    {
        return false;
    }
    
    Feat->Shape->ShpFileSize = (i32)FileSize;
    isz ContentLength = ((isz)EndOfFile - (isz)Multipatch) / 2;
    ShpRecord->ContentLength = FlipEndian32(ContentLength);
    Header->FileLength = FlipEndian32(FileLength);
    
    shx_record* ShxRecord = (shx_record*)Feat->ShxRecord;
    ShxRecord->ContentLength = ShpRecord->ContentLength;
    
    Multipatch->Type = Header->Type;
    Multipatch->BBox = BBox;
    Multipatch->NumPoints += NumPoints;
    
    Header->BBox.XMin = Min(Header->BBox.XMin, BBox.XMin);
    Header->BBox.YMin = Min(Header->BBox.YMin, BBox.YMin);
    Header->BBox.XMax = Max(Header->BBox.XMax, BBox.XMax);
    Header->BBox.YMax = Max(Header->BBox.YMax, BBox.YMax);
    
    shx_header* ShxHeader = (shx_header*)Feat->Shape->ShxFilePtr;
    ShxHeader->BBox = Header->BBox;
    
    return true;
}

external bool
AddAttrString(shp_feature* Feat, char* String, usz StringSize, i32 FieldIdx)
{
    target_fd Field = _GetFDByIdx(Feat->Shape->DbfFilePtr, FieldIdx);
    if (Field.FD->Type == DbfField_Char)
    {
        u8* WritePtr = Feat->DbfRecord + 1 + Field.Offset;
        if (StringSize == -1) StringSize = strlen(String);
        return CopyData(WritePtr, Field.FD->FieldLen, String, StringSize);
    }
    return false;
}

external bool
AddAttrInteger(shp_feature* Feat, isz Integer, i32 FieldIdx)
{
    target_fd Field = _GetFDByIdx(Feat->Shape->DbfFilePtr, FieldIdx);
    if (Field.FD->Type == DbfField_Numeric)
    {
        usz IntSize = NumberOfDigits(Integer) + (Integer < 0);
        if (IntSize > Field.FD->FieldLen) return false;
        
        u8* WritePtr = Feat->DbfRecord + 1 + Field.Offset;
        for (usz Count = 0; Count < Field.FD->FieldLen - IntSize; Count++)
        {
            *WritePtr++ = 0x20;
        }
        
        string Write = String(WritePtr, 0, IntSize, EC_ASCII);
        return AppendIntToString(Integer, &Write);
    }
    return false;
}

external bool
AddAttrReal(shp_feature* Feat, f64 Real, i32 FieldIdx)
{
    target_fd Field = _GetFDByIdx(Feat->Shape->DbfFilePtr, FieldIdx);
    u8* WritePtr = Feat->DbfRecord + 1 + Field.Offset;
    
    if (Field.FD->Type == DbfField_Numeric)
    {
        isz Integer = (isz)Real;
        usz IntSize = NumberOfDigits(Integer);
        if (IntSize > Field.FD->FieldLen) return false;
        
        usz RemainingSize = Field.FD->FieldLen - IntSize;
        usz RealSize = IntSize + Min(RemainingSize, Field.FD->Decimals + 2); // OBS: Extra bytes for "." and sign.
        usz BlankSize = Field.FD->FieldLen - RealSize;
        for (usz Count = 0; Count < BlankSize; Count++)
        {
            *WritePtr++ = 0x20;
        }
        
        *WritePtr++ = (Real < 0) ? '-' : ' ';
        string Write = String(WritePtr, 0, RealSize-1, EC_ASCII);
        return AppendFloatToString(Abs(Real), RealSize - IntSize - 2, false, &Write);
    }
    else if (Field.FD->Type == DbfField_Float)
    {
        usz NonDecimalSize = 8;
        usz DecimalSize = Min(Field.FD->Decimals, Field.FD->FieldLen - NonDecimalSize);
        usz RealSize = NonDecimalSize + DecimalSize;
        usz BlankSize = Field.FD->FieldLen - RealSize;
        for (usz Count = 0; Count < BlankSize; Count++)
        {
            *WritePtr++ = 0x20;
        }
        
        *WritePtr++ = (Real < 0) ? '-' : ' ';
        string Write = String(WritePtr, 0, RealSize-1, EC_ASCII);
        return AppendFloatToString(Abs(Real), DecimalSize, true, &Write);
    }
    return false;
}

external bool
AddAttrBoolean(shp_feature* Feat, bool Boolean, i32 FieldIdx)
{
    target_fd Field = _GetFDByIdx(Feat->Shape->DbfFilePtr, FieldIdx);
    if (Field.FD->Type == DbfField_Logical)
    {
        u8* WritePtr = Feat->DbfRecord + 1 + Field.Offset;
        WritePtr[0] = Boolean ? 'T' : 'F';
        return true;
    }
    return false;
}

external bool
AddAttrDate(shp_feature* Feat, usz Year, usz Month, usz Day, i32 FieldIdx)
{
    target_fd Field = _GetFDByIdx(Feat->Shape->DbfFilePtr, FieldIdx);
    if (Field.FD->Type == DbfField_Date
        && Year <= 9999
        && Month <= 12
        && Day <= 31)
    {
        u8* WritePtr = Feat->DbfRecord + 1 + Field.Offset;
        string Write = String(WritePtr, 0, 8, EC_ASCII);
        
        if (Year < 1900) { Year = 1899; Month = 12; Day = 31; }
        AppendIntToString(Year, &Write);
        if (Month < 10) Write.Base[Write.WriteCur++] = '0';
        AppendIntToString(Month, &Write);
        if (Day < 10) Write.Base[Write.WriteCur++] = '0';
        AppendIntToString(Day, &Write);
        
        return true;
    }
    return false;
}
