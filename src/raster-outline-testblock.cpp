//================================
// Structs and defines
//================================

enum edge_type
{
    EdgeType_None,
    EdgeType_TopLeft,
    EdgeType_TopRight,
    EdgeType_BottomLeft,
    EdgeType_BottomRight,
    EdgeType_TopLeftBottomRight,
    EdgeType_TopRightBottomLeft
};

typedef edge_type (*test_block)(u8*, u8*, u32, u32, double);

internal edge_type
GetBlockEdges(bool TLValue, bool TRValue, bool BLValue, bool BRValue)
{
    int ValueCount = TLValue + TRValue + BLValue + BRValue;
    if (ValueCount == 1)
    {
        if (TLValue) return EdgeType_TopLeft;
        if (TRValue) return EdgeType_TopRight;
        if (BLValue) return EdgeType_BottomLeft;
        if (BRValue) return EdgeType_BottomRight;
    }
    if (ValueCount == 2)
    {
        if (TLValue && BRValue) return EdgeType_TopLeftBottomRight;
        if (TRValue && BLValue) return EdgeType_TopRightBottomLeft;
    }
    if (ValueCount == 3)
    {
        if (!TLValue) return EdgeType_TopLeft;
        if (!TRValue) return EdgeType_TopRight;
        if (!BLValue) return EdgeType_BottomLeft;
        if (!BRValue) return EdgeType_BottomRight;
    }
    return EdgeType_None;
}

//================================
// Test for equal
//================================

#define TEST_FOR_EQUAL do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] == Value; \
TRValue &= Top[1] == Value; \
BLValue &= Bottom[0] == Value; \
BRValue &= Bottom[1] == Value; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 Value = (u8)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 Value = (u16)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 Value = (u32)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 Value = (f32)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 Value = (f64)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for not equal
//================================

#define TEST_FOR_NOTEQUAL do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] != Value; \
TRValue &= Top[1] != Value; \
BLValue &= Bottom[0] != Value; \
BRValue &= Bottom[1] != Value; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 Value = (u8)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_NOTEQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 Value = (u16)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_NOTEQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 Value = (u32)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_NOTEQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 Value = (f32)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

internal edge_type
TestF64NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _Value)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 Value = (f64)_Value;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_NOTEQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test selection
//================================

global test_block TestCallbacks[2][5] = {
    { TestU8Equal, TestU16Equal, TestU32Equal, TestF32Equal, TestF64Equal },
    { TestU8NotEqual, TestU16NotEqual, TestU32NotEqual, TestF32NotEqual, TestF64NotEqual}};

internal test_block
GetTestBlockCallback(GDALDataType DType, test_type TestType)
{
    int Offset = 0; // GDT_Byte
    switch (DType)
    {
        case GDT_UInt16:
        case GDT_Int16:   Offset = 1; break;
        case GDT_UInt32:
        case GDT_Int32:   Offset = 2; break;
        case GDT_Float32: Offset = 3; break;
        case GDT_Float64: Offset = 4; break;
    }
    test_block Result = TestCallbacks[TestType][Offset];
    return Result;
}
