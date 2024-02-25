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
    EdgeType_Cross,
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
    else if (ValueCount == 2)
    {
        if ((TLValue + BRValue) != 1) return EdgeType_Cross;
    }
    else if (ValueCount == 3)
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
TLValue &= Top[0] == ValueA; \
TRValue &= Top[1] == ValueA; \
BLValue &= Bottom[0] == ValueA; \
BRValue &= Bottom[1] == ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
            double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
             double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
             double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
             double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64Equal(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
             double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for not equal
//================================

internal edge_type
TestU8NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

internal edge_type
TestU16NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

internal edge_type
TestU32NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

internal edge_type
TestF32NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

internal edge_type
TestF64NotEqual(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_EQUAL;
    
    return GetBlockEdges(!TLValue, !TRValue, !BLValue, !BRValue);
}

//================================
// Test for bigger than
//================================

#define TEST_FOR_BIGGERTHAN do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] > ValueA; \
TRValue &= Top[1] > ValueA; \
BLValue &= Bottom[0] > ValueA; \
BRValue &= Bottom[1] > ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8BiggerThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                 double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGERTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16BiggerThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                  double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGERTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32BiggerThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                  double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGERTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32BiggerThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                  double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGERTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64BiggerThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                  double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGERTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for bigger or equal to
//================================

#define TEST_FOR_BIGGEROREQUALTO do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] >= ValueA; \
TRValue &= Top[1] >= ValueA; \
BLValue &= Bottom[0] >= ValueA; \
BRValue &= Bottom[1] >= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8BiggerOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                      double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGEROREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16BiggerOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                       double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGEROREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32BiggerOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                       double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGEROREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32BiggerOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                       double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGEROREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64BiggerOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                       double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BIGGEROREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for less than
//================================

#define TEST_FOR_LESSTHAN do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] < ValueA; \
TRValue &= Top[1] < ValueA; \
BLValue &= Bottom[0] < ValueA; \
BRValue &= Bottom[1] < ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8LessThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16LessThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32LessThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32LessThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64LessThan(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSTHAN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for less or equal to.
//================================

#define TEST_FOR_LESSOREQUALTO do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= Top[0] <= ValueA; \
TRValue &= Top[1] <= ValueA; \
BLValue &= Bottom[0] <= ValueA; \
BRValue &= Bottom[1] <= ValueA; \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8LessOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                    double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSOREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU16LessOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                     double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSOREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32LessOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                     double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSOREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32LessOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                     double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSOREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64LessOrEqualTo(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
                     double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_LESSOREQUALTO;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test for between
//================================

#define TEST_FOR_BETWEEN do { \
for (usz Count = 0; Count < BandCount; Count++) \
{ \
TLValue &= (Top[0] >= ValueA) && (Top[0] <= ValueB); \
TRValue &= (Top[1] >= ValueA) && (Top[1] <= ValueB); \
BLValue &= (Bottom[0] >= ValueA) && (Bottom[0] <= ValueB); \
BRValue &= (Bottom[1] >= ValueA) && (Bottom[1] <= ValueB); \
Top += InspectWidth; \
Bottom += InspectWidth; \
} \
} while (0);

internal edge_type
TestU8Between(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
              double _ValueA, double _ValueB)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 ValueA = (u8)_ValueA;
    u8 ValueB = (u8)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BETWEEN;
    
    return GetBlockEdges(TLValue==0, TRValue==0, BLValue==0, BRValue==0);
}

internal edge_type
TestU16Between(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 ValueA = (u16)_ValueA;
    u16 ValueB = (u16)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BETWEEN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestU32Between(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 ValueA = (u32)_ValueA;
    u32 ValueB = (u32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BETWEEN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF32Between(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 ValueA = (f32)_ValueA;
    f32 ValueB = (f32)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BETWEEN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

internal edge_type
TestF64Between(u8* _TopRow, u8* _BottomRow, u32 InspectWidth, u32 BandCount,
               double _ValueA, double _ValueB)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 ValueA = (f64)_ValueA;
    f64 ValueB = (f64)_ValueB;
    
    bool TLValue = true, TRValue = true, BLValue = true, BRValue = true;
    TEST_FOR_BETWEEN;
    
    return GetBlockEdges(TLValue, TRValue, BLValue, BRValue);
}

//================================
// Test selection
//================================

global test_block TestCallbacks[][5] = {
    { TestU8Equal, TestU16Equal, TestU32Equal, TestF32Equal, TestF64Equal },
    { TestU8NotEqual, TestU16NotEqual, TestU32NotEqual, TestF32NotEqual, TestF64NotEqual},
    { TestU8BiggerThan, TestU16BiggerThan, TestU32BiggerThan, TestF32BiggerThan, TestF64BiggerThan },
    { TestU8BiggerOrEqualTo, TestU16BiggerOrEqualTo, TestU32BiggerOrEqualTo, TestF32BiggerOrEqualTo, TestF64BiggerOrEqualTo },
    { TestU8LessThan, TestU16LessThan, TestU32LessThan, TestF32LessThan, TestF64LessThan },
    { TestU8LessOrEqualTo, TestU16LessOrEqualTo, TestU32LessOrEqualTo, TestF32LessOrEqualTo, TestF64LessOrEqualTo },
    { TestU8Between, TestU16Between, TestU32Between, TestF32Between, TestF64Between }};

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
