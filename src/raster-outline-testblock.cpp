//================================
// Structs and defines
//================================

enum edge_type
{
    EdgeType_None = 0,
    
    EdgeType_TopLeft = 1,
    EdgeType_TopRight = 2,
    EdgeType_BottomLeft = 3,
    EdgeType_BottomRight = 4,
    
    EdgeType_TopLeftInv = 0x11,
    EdgeType_TopRightInv = 0x12,
    EdgeType_BottomLeftInv = 0x13,
    EdgeType_BottomRightInv = 0x14,
    
    EdgeType_TopLeftBottomRight = 0x20,
    EdgeType_TopRightBottomLeft = 0x21
};

typedef edge_type (*test_block)(u8*, u8*, u32, u32, double, double);

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
        if (!TLValue) return EdgeType_TopLeftInv;
        if (!TRValue) return EdgeType_TopRightInv;
        if (!BLValue) return EdgeType_BottomLeftInv;
        if (!BRValue) return EdgeType_BottomRightInv;
    }
    return EdgeType_None;
}

//================================
// Test for equal (all)
//================================

#define TEST_FOR_EQUAL_ALL \
bool TLValue = true, TRValue = true, BLValue = true, BRValue = true; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] == ValueA; \
TRValue &= Top[1] == ValueA; \
BLValue &= Bottom[0] == ValueA; \
BRValue &= Bottom[1] == ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8EqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_EQUAL_ALL;
}

internal edge_type
TestU16EqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_EQUAL_ALL;
}

internal edge_type
TestU32EqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_EQUAL_ALL;
}

internal edge_type
TestF32EqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_EQUAL_ALL;
}

internal edge_type
TestF64EqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_EQUAL_ALL;
}

//================================
// Test for equal (any)
//================================

#define TEST_FOR_EQUAL_ANY \
bool TLValue = false, TRValue = false, BLValue = false, BRValue = false; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue |= Top[0] == ValueA; \
TRValue |= Top[1] == ValueA; \
BLValue |= Bottom[0] == ValueA; \
BRValue |= Bottom[1] == ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8EqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_EQUAL_ANY;
}

internal edge_type
TestU16EqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_EQUAL_ANY;
}

internal edge_type
TestU32EqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_EQUAL_ANY;
}

internal edge_type
TestF32EqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_EQUAL_ANY;
}

internal edge_type
TestF64EqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_EQUAL_ANY;
}

//================================
// Test for not equal (all)
//================================

#define TEST_FOR_NOTEQUAL_ALL \
bool TLValue = true, TRValue = true, BLValue = true, BRValue = true; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] == ValueA; \
TRValue &= Top[1] == ValueA; \
BLValue &= Bottom[0] == ValueA; \
BRValue &= Bottom[1] == ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);

internal edge_type
TestU8NotEqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_NOTEQUAL_ALL;
}

internal edge_type
TestU16NotEqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_NOTEQUAL_ALL;
}

internal edge_type
TestU32NotEqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_NOTEQUAL_ALL;
}

internal edge_type
TestF32NotEqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_NOTEQUAL_ALL;
}

internal edge_type
TestF64NotEqualAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_NOTEQUAL_ALL;
}

//================================
// Test for not equal (any)
//================================

#define TEST_FOR_NOTEQUAL_ANY \
bool TLValue = false, TRValue = false, BLValue = false, BRValue = false; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue |= Top[0] == ValueA; \
TRValue |= Top[1] == ValueA; \
BLValue |= Bottom[0] == ValueA; \
BRValue |= Bottom[1] == ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);

internal edge_type
TestU8NotEqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_NOTEQUAL_ANY;
}

internal edge_type
TestU16NotEqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_NOTEQUAL_ANY;
}

internal edge_type
TestU32NotEqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_NOTEQUAL_ANY;
}

internal edge_type
TestF32NotEqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_NOTEQUAL_ANY;
}

internal edge_type
TestF64NotEqualAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_NOTEQUAL_ANY;
}

//================================
// Test for more than (all)
//================================

#define TEST_FOR_MORETHAN_ALL \
bool TLValue = true, TRValue = true, BLValue = true, BRValue = true; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] >= ValueA; \
TRValue &= Top[1] >= ValueA; \
BLValue &= Bottom[0] >= ValueA; \
BRValue &= Bottom[1] >= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8MoreThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_MORETHAN_ALL;
}

internal edge_type
TestU16MoreThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_MORETHAN_ALL;
}

internal edge_type
TestU32MoreThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_MORETHAN_ALL;
}

internal edge_type
TestF32MoreThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_MORETHAN_ALL;
}

internal edge_type
TestF64MoreThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_MORETHAN_ALL;
}

//================================
// Test for more than (any)
//================================

#define TEST_FOR_MORETHAN_ANY \
bool TLValue = false, TRValue = false, BLValue = false, BRValue = false; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue |= Top[0] >= ValueA; \
TRValue |= Top[1] >= ValueA; \
BLValue |= Bottom[0] >= ValueA; \
BRValue |= Bottom[1] >= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8MoreThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_MORETHAN_ANY;
}

internal edge_type
TestU16MoreThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_MORETHAN_ANY;
}

internal edge_type
TestU32MoreThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_MORETHAN_ANY;
}

internal edge_type
TestF32MoreThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_MORETHAN_ANY;
}

internal edge_type
TestF64MoreThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_MORETHAN_ANY;
}

//================================
// Test for less than (all)
//================================

#define TEST_FOR_LESSTHAN_ALL \
bool TLValue = true, TRValue = true, BLValue = true, BRValue = true; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] <= ValueA; \
TRValue &= Top[1] <= ValueA; \
BLValue &= Bottom[0] <= ValueA; \
BRValue &= Bottom[1] <= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8LessThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_LESSTHAN_ALL;
}

internal edge_type
TestU16LessThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_LESSTHAN_ALL;
}

internal edge_type
TestU32LessThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_LESSTHAN_ALL;
}

internal edge_type
TestF32LessThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_LESSTHAN_ALL;
}

internal edge_type
TestF64LessThanAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_LESSTHAN_ALL;
}

//================================
// Test for less than (any)
//================================

#define TEST_FOR_LESSTHAN_ANY \
bool TLValue = false, TRValue = false, BLValue = false, BRValue = false; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue |= Top[0] <= ValueA; \
TRValue |= Top[1] <= ValueA; \
BLValue |= Bottom[0] <= ValueA; \
BRValue |= Bottom[1] <= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8LessThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    TEST_FOR_LESSTHAN_ANY;
}

internal edge_type
TestU16LessThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    TEST_FOR_LESSTHAN_ANY;
}

internal edge_type
TestU32LessThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    
    TEST_FOR_LESSTHAN_ANY;
}

internal edge_type
TestF32LessThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    
    TEST_FOR_LESSTHAN_ANY;
}

internal edge_type
TestF64LessThanAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    
    TEST_FOR_LESSTHAN_ANY;
}

//================================
// Test for between (all)
//================================

#define TEST_FOR_BETWEEN_ALL \
bool TLValue = true, TRValue = true, BLValue = true, BRValue = true; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= (Top[0] >= ValueA) && (Top[0] <= ValueB); \
TRValue &= (Top[1] >= ValueA) && (Top[1] <= ValueB); \
BLValue &= (Bottom[0] >= ValueA) && (Bottom[0] <= ValueB); \
BRValue &= (Bottom[1] >= ValueA) && (Bottom[1] <= ValueB); \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8BetweenAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    TEST_FOR_BETWEEN_ALL;
}

internal edge_type
TestU16BetweenAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    TEST_FOR_BETWEEN_ALL;
}

internal edge_type
TestU32BetweenAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    TEST_FOR_BETWEEN_ALL;
}

internal edge_type
TestF32BetweenAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    TEST_FOR_BETWEEN_ALL;
}

internal edge_type
TestF64BetweenAll(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    TEST_FOR_BETWEEN_ALL;
}

//================================
// Test for between (any)
//================================

#define TEST_FOR_BETWEEN_ANY \
bool TLValue = false, TRValue = false, BLValue = false, BRValue = false; \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue |= (Top[0] >= ValueA) && (Top[0] <= ValueB); \
TRValue |= (Top[1] >= ValueA) && (Top[1] <= ValueB); \
BLValue |= (Bottom[0] >= ValueA) && (Bottom[0] <= ValueB); \
BRValue |= (Bottom[1] >= ValueA) && (Bottom[1] <= ValueB); \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);

internal edge_type
TestU8BetweenAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    TEST_FOR_BETWEEN_ANY;
}

internal edge_type
TestU16BetweenAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    TEST_FOR_BETWEEN_ANY;
}

internal edge_type
TestU32BetweenAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    TEST_FOR_BETWEEN_ANY;
}

internal edge_type
TestF32BetweenAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    TEST_FOR_BETWEEN_ANY;
}

internal edge_type
TestF64BetweenAny(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount, double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    TEST_FOR_BETWEEN_ANY;
}

//================================
// Test selection
//================================

global test_block TestCallbacks[][5] = {
    { TestU8EqualAll, TestU16EqualAll, TestU32EqualAll, TestF32EqualAll, TestF64EqualAll },
    { TestU8EqualAny, TestU16EqualAny, TestU32EqualAny, TestF32EqualAny, TestF64EqualAny },
    { TestU8NotEqualAll, TestU16NotEqualAll, TestU32NotEqualAll, TestF32NotEqualAll, TestF64NotEqualAll },
    { TestU8NotEqualAny, TestU16NotEqualAny, TestU32NotEqualAny, TestF32NotEqualAny, TestF64NotEqualAny },
    { TestU8MoreThanAll, TestU16MoreThanAll, TestU32MoreThanAll, TestF32MoreThanAll, TestF64MoreThanAll },
    { TestU8MoreThanAny, TestU16MoreThanAny, TestU32MoreThanAny, TestF32MoreThanAny, TestF64MoreThanAny },
    { TestU8LessThanAll, TestU16LessThanAll, TestU32LessThanAll, TestF32LessThanAll, TestF64LessThanAll },
    { TestU8LessThanAny, TestU16LessThanAny, TestU32LessThanAny, TestF32LessThanAny, TestF64LessThanAny },
    { TestU8BetweenAll, TestU16BetweenAll, TestU32BetweenAll, TestF32BetweenAll, TestF64BetweenAll },
    { TestU8BetweenAny, TestU16BetweenAny, TestU32BetweenAny, TestF32BetweenAny, TestF64BetweenAny }};

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
