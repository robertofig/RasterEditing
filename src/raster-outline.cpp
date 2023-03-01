#include "tinybase-platform.h"

//================================
// Structs and defines
//================================

#define EDGE_DATA_START_SIZE Kilobyte(64)
#define RING_DATA_START_SIZE Kilobyte(64)

enum edge_type
{
    EdgeType_TopLeft = 0,
    EdgeType_TopRight = 1,
    EdgeType_BottomLeft = 2,
    EdgeType_BottomRight = 3,
    EdgeType_None = 4
};

struct edge
{
    edge_type Type;
    int Row;
    int Col;
    u32 Above;
    u32 Below;
    u32 SortedIdx;
};

enum line_dir
{
    LineDir_Left,
    LineDir_Right,
    LineDir_Down,
    LineDir_Up,
    LineDir_None
};

typedef edge_type (*test_block)(u8*, u8*, double);
struct arena_info
{
    u32 LineSweepSize;
    u8* FirstLine;
    u8* SecondLine;
    test_block TestBlock;
    double NoData;
    
    u32 ColHashTableSize;
    u32* ColHashTable;
    
    u32 EdgeDataSize;
    u32 EdgeCount;
    edge* EdgeList;
    u32* SortedEdges;
};

struct tree_node
{
    f64 BBoxArea;
    ring_info* Ring;
    
    tree_node* Parent;
    tree_node* Sibling;
    tree_node* Child;
};

//================================
// Functions
//================================

internal edge_type
_TestBlockU8(u8* _TopRow, u8* _BottomRow, double _NoData)
{
    u8* Top = _TopRow;
    u8* Bottom = _BottomRow;
    u8 NoData = (u8)_NoData;
    
    bool TLNoData = Top[0] == NoData;
    bool TRNoData = Top[1] == NoData;
    bool BLNoData = Bottom[0] == NoData;
    bool BRNoData = Bottom[1] == NoData;
    
    int NoDataCount = TLNoData + TRNoData + BLNoData + BRNoData;
    if (NoDataCount == 1)
    {
        if (TLNoData) return EdgeType_TopLeft;
        if (TRNoData) return EdgeType_TopRight;
        if (BLNoData) return EdgeType_BottomLeft;
        if (BRNoData) return EdgeType_BottomRight;
    }
    if (NoDataCount == 3)
    {
        if (!TLNoData) return EdgeType_TopLeft;
        if (!TRNoData) return EdgeType_TopRight;
        if (!BLNoData) return EdgeType_BottomLeft;
        if (!BRNoData) return EdgeType_BottomRight;
    }
    
    return EdgeType_None;
}

internal edge_type
_TestBlockU16(u8* _TopRow, u8* _BottomRow, double _NoData)
{
    u16* Top = (u16*)_TopRow;
    u16* Bottom = (u16*)_BottomRow;
    u16 NoData = (u16)_NoData;
    
    bool TLNoData = Top[0] == NoData;
    bool TRNoData = Top[1] == NoData;
    bool BLNoData = Bottom[0] == NoData;
    bool BRNoData = Bottom[1] == NoData;
    
    if ( TLNoData && !TRNoData && !BLNoData && !BRNoData) return EdgeType_TopLeft;
    if (!TLNoData &&  TRNoData && !BLNoData && !BRNoData) return EdgeType_TopRight;
    if (!TLNoData && !TRNoData &&  BLNoData && !BRNoData) return EdgeType_BottomLeft;
    if (!TLNoData && !TRNoData && !BLNoData &&  BRNoData) return EdgeType_BottomRight;
    
    return EdgeType_None;
}

internal edge_type
_TestBlockU32(u8* _TopRow, u8* _BottomRow, double _NoData)
{
    u32* Top = (u32*)_TopRow;
    u32* Bottom = (u32*)_BottomRow;
    u32 NoData = (u32)_NoData;
    
    bool TLNoData = Top[0] == NoData;
    bool TRNoData = Top[1] == NoData;
    bool BLNoData = Bottom[0] == NoData;
    bool BRNoData = Bottom[1] == NoData;
    
    if ( TLNoData && !TRNoData && !BLNoData && !BRNoData) return EdgeType_TopLeft;
    if (!TLNoData &&  TRNoData && !BLNoData && !BRNoData) return EdgeType_TopRight;
    if (!TLNoData && !TRNoData &&  BLNoData && !BRNoData) return EdgeType_BottomLeft;
    if (!TLNoData && !TRNoData && !BLNoData &&  BRNoData) return EdgeType_BottomRight;
    
    return EdgeType_None;
}

internal edge_type
_TestBlockF32(u8* _TopRow, u8* _BottomRow, double _NoData)
{
    f32* Top = (f32*)_TopRow;
    f32* Bottom = (f32*)_BottomRow;
    f32 NoData = (f32)_NoData;
    
    bool TLNoData = Top[0] == NoData;
    bool TRNoData = Top[1] == NoData;
    bool BLNoData = Bottom[0] == NoData;
    bool BRNoData = Bottom[1] == NoData;
    
    if ( TLNoData && !TRNoData && !BLNoData && !BRNoData) return EdgeType_TopLeft;
    if (!TLNoData &&  TRNoData && !BLNoData && !BRNoData) return EdgeType_TopRight;
    if (!TLNoData && !TRNoData &&  BLNoData && !BRNoData) return EdgeType_BottomLeft;
    if (!TLNoData && !TRNoData && !BLNoData &&  BRNoData) return EdgeType_BottomRight;
    
    return EdgeType_None;
}

internal edge_type
_TestBlockF64(u8* _TopRow, u8* _BottomRow, double _NoData)
{
    f64* Top = (f64*)_TopRow;
    f64* Bottom = (f64*)_BottomRow;
    f64 NoData = (f64)_NoData;
    
    bool TLNoData = Top[0] == NoData;
    bool TRNoData = Top[1] == NoData;
    bool BLNoData = Bottom[0] == NoData;
    bool BRNoData = Bottom[1] == NoData;
    
    if ( TLNoData && !TRNoData && !BLNoData && !BRNoData) return EdgeType_TopLeft;
    if (!TLNoData &&  TRNoData && !BLNoData && !BRNoData) return EdgeType_TopRight;
    if (!TLNoData && !TRNoData &&  BLNoData && !BRNoData) return EdgeType_BottomLeft;
    if (!TLNoData && !TRNoData && !BLNoData &&  BRNoData) return EdgeType_BottomRight;
    
    return EdgeType_None;
}

internal usz
GetDTypeSize(GDALDataType DType)
{
    switch (DType)
    {
        case GDT_Byte:    return 1;
        case GDT_UInt16:
        case GDT_Int16:   return 2;
        case GDT_UInt32:
        case GDT_Int32:
        case GDT_Float32: return 4;
        case GDT_Float64: return 8;
        default:          return 0;
    }
}

internal test_block
GetTestBlockCallback(GDALDataType DType)
{
    switch (DType)
    {
        case GDT_Byte:    return _TestBlockU8 ;
        case GDT_UInt16:
        case GDT_Int16:   return _TestBlockU16 ;
        case GDT_UInt32:
        case GDT_Int32:   return _TestBlockU32;
        case GDT_Float32: return _TestBlockF32;
        case GDT_Float64: return _TestBlockF64;
        default:          return NULL;
    }
}

internal void
SetNoDataLine(u8* Line, int Width, double NoData, GDALDataType DType)
{
    switch (DType)
    {
        default:
        case GDT_Byte:
        {
            u8 Value = (u8)NoData;
            for (u8* Ptr = Line; Ptr < Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_UInt16:
        {
            u16 Value = (u16)NoData;
            for (u16* Ptr = (u16*)Line; Ptr < (u16*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Int16:
        {
            i16 Value = (i16)NoData;
            for (i16* Ptr = (i16*)Line; Ptr < (i16*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_UInt32:
        {
            u32 Value = (u32)NoData;
            for (u32* Ptr = (u32*)Line; Ptr < (u32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Int32:
        {
            i32 Value = (i32)NoData;
            for (i32* Ptr = (i32*)Line; Ptr < (i32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Float32:
        {
            f32 Value = (f32)NoData;
            for (f32* Ptr = (f32*)Line; Ptr < (f32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Float64:
        {
            f64 Value = (f64)NoData;
            for (f64* Ptr = (f64*)Line; Ptr < (f64*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
    }
}

internal bool
ProcessSweepLine(buffer* Arena, arena_info** InfoPtr, int Row)
{
    arena_info* Info = (arena_info*)Arena->Base;
    for (int Col = 0; Col < Info->LineSweepSize-1; Col++)
    {
        edge_type Type = Info->TestBlock(Info->FirstLine + Col, Info->SecondLine + Col, Info->NoData);
        if (Type != EdgeType_None)
        {
            u32 RequiredSize = sizeof(edge) + sizeof(u32); // This u32 is for the sorted index.
            if ((Arena->Size - Arena->WriteCur) < RequiredSize)
            {
                usz NewAllocSize = Arena->Size * 2; // Doubles size so it won't keep reallocing all the time.
                u8* NewAlloc = (u8*)GetMemory(NewAllocSize, 0, MEM_WRITE);
                if (!NewAlloc)
                {
                    FreeMemory(Arena->Base);
                    return false;
                }
                CopyData(NewAlloc, Arena->WriteCur, Arena->Base, Arena->WriteCur);
                FreeMemory(Arena->Base);
                
                Arena->Base = NewAlloc;
                Arena->Size = Align(NewAllocSize, gSysInfo.PageSize);
                
                Info = *InfoPtr = (arena_info*)NewAlloc;
                Info->FirstLine = (u8*)&Info[1];
                Info->SecondLine = Info->FirstLine + Info->LineSweepSize;
                Info->ColHashTable = (u32*)(Info->SecondLine + Info->LineSweepSize);
                Info->EdgeList = (edge*)((u8*)Info->ColHashTable + Info->ColHashTableSize);
            }
            
            u32 NewEdgeIdx = Info->EdgeCount++;
            edge* NewEdge = PushStruct(Arena, edge);
            *NewEdge = { Type, Row, Col, 0, 0 };
            
            u32 AboveEdgeIdx = Info->ColHashTable[Col];
            if (AboveEdgeIdx > 0)
            {
                edge* AboveEdge = &Info->EdgeList[AboveEdgeIdx];
                NewEdge->Above = AboveEdgeIdx;
                AboveEdge->Below = NewEdgeIdx;
            }
            Info->ColHashTable[Col] = NewEdgeIdx;
        }
    }
    
    return true;
}

internal line_dir
GetLineDirection(line_dir PrevDir, edge* Edge)
{
    switch (Edge->Type)
    {
        case EdgeType_TopLeft:     return (PrevDir == LineDir_Right) ? LineDir_Up   : LineDir_Left;
        case EdgeType_TopRight:    return (PrevDir == LineDir_Left)  ? LineDir_Up   : LineDir_Right;
        case EdgeType_BottomLeft:  return (PrevDir == LineDir_Right) ? LineDir_Down : LineDir_Left;
        case EdgeType_BottomRight: return (PrevDir == LineDir_Left)  ? LineDir_Down : LineDir_Right;
        default:                   return LineDir_None;
    }
}

internal void
WriteVertexPair(ring_info* Ring, double* Affine, edge* EdgeList, u32* EdgeIdx, bbox2* BBox)
{
    edge* FirstEdge = &EdgeList[EdgeIdx[0]];
    edge* SecondEdge = &EdgeList[EdgeIdx[1]];
    
    f64 X1 = Affine[0] + Affine[1] * FirstEdge->Col;
    f64 Y1 = Affine[3] + Affine[5] * FirstEdge->Row;
    Ring->Vertices[Ring->NumVertices++] = V2(X1, Y1);
    
    f64 X2 = Affine[0] + Affine[1] * SecondEdge->Col;
    f64 Y2 = Affine[3] + Affine[5] * SecondEdge->Row;
    Ring->Vertices[Ring->NumVertices++]= V2(X2, Y2);
    
    BBox->Min.X = Min(Min(X1, X2), BBox->Min.X);
    BBox->Min.Y = Min(Min(Y1, Y2), BBox->Min.Y);
    BBox->Max.X = Max(Max(X1, X2), BBox->Max.X);
    BBox->Max.Y = Max(Max(Y1, Y2), BBox->Max.Y);
}

internal ring_info*
GetNextRingInfo(ring_info* Ring)
{
    ring_info* Result = (ring_info*)(((v2*)&Ring[1]) + Ring->NumVertices);
    return Result;
}

internal bool
CheckCollision(v2 Test, v2 BP0, v2 BP1, v2 BP2, v2 BP3)
{
    if (BP1.X == BP2.X) // Vertical line
    {
        f64 YMin = Min(BP1.Y, BP2.Y);
        f64 YMax = Max(BP1.Y, BP2.Y);
        return (Test.X < BP1.X && Test.Y >= YMin && Test.Y <= YMax);
    }
    else if (BP1.Y == Test.Y) // Collinear Horizontal line
    {
        f64 XMin = Min(BP1.X, BP2.X);
        if (Test.X < XMin)  // Hits ray to the right.
        {
            return ((BP0.Y < BP1.Y && BP2.Y < BP3.Y) || (BP0.Y > BP1.Y && BP2.Y > BP3.Y));
        }
    }
    return false;
}

internal bool
IsRingInsideRing(ring_info* A, ring_info* B)
{
    if (A != B)
    {
        v2 Test = A->Vertices[0];
        usz RayCount = 0;
        
        // BP1-BP2 is the test line, BP0-BP1 is the line before and BP1-BP2 is the line after.
        v2 BP0 = B->Vertices[B->NumVertices-2];
        v2 BP1 = B->Vertices[0];
        v2 BP2 = B->Vertices[1];
        
        for (u32 BIdx = 2; BIdx < B->NumVertices; BIdx++)
        {
            v2 BP3 = B->Vertices[BIdx];
            RayCount += CheckCollision(Test, BP0, BP1, BP2, BP3);
            BP0 = BP1;
            BP1 = BP2;
            BP2 = BP3;
        }
        v2 BP3 = B->Vertices[1];
        RayCount += CheckCollision(Test, BP0, BP1, BP2, BP3);
        
        return RayCount % 2;
    }
    
    return false;
}

internal poly_info
RasterToOutline(GDALDatasetH DS)
{
    poly_info Poly = {0}, EmptyPoly = {0};
    
    timing Trace;
    StartTiming(&Trace);
    
    GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
    GDALDataType DType = GDALGetRasterDataType(Band);
    int Width = GDALGetRasterXSize(DS);
    int Height = GDALGetRasterYSize(DS);
    int Bands = GDALGetRasterCount(DS);
    
    int InspectWidth = Width + 2; // One extra pixel on the left and right, to guard from overflow.
    int InspectHeight = Height + 2; // One extra row on top and bottom, to guard from overflow.
    usz DTypeSize = GetDTypeSize(DType);
    
    double Affine[6];
    GDALGetGeoTransform(DS, Affine);
    
    int RasterHasNoData = 0;
    double NoData = GDALGetRasterNoDataValue(Band, &RasterHasNoData); // Check if DType isn't i64 or u64.
    
    if (RasterHasNoData)
    {
        // Prepare memory arena.
        
        u32 ColHashTableSize = Width * sizeof(u32);
        u32 LineSweepSize = InspectWidth * DTypeSize;
        u32 EdgeDataSize = EDGE_DATA_START_SIZE;
        
        usz ArenaSize = sizeof(arena_info) + (LineSweepSize * 2) + ColHashTableSize + EdgeDataSize;
        void* ArenaMem = GetMemory(ArenaSize, 0, MEM_WRITE);
        if (!ArenaMem)
        {
            return EmptyPoly;
        }
        buffer Arena = Buffer(ArenaMem, 0, ArenaSize);
        arena_info* Info = PushStruct(&Arena, arena_info);
        
        Info->LineSweepSize = LineSweepSize;
        Info->FirstLine = PushSize(&Arena, LineSweepSize, u8);
        Info->SecondLine = PushSize(&Arena, LineSweepSize, u8);
        Info->TestBlock = GetTestBlockCallback(DType);
        Info->NoData = NoData;
        Info->ColHashTableSize = ColHashTableSize;
        Info->ColHashTable = PushSize(&Arena, ColHashTableSize, u32);
        Info->EdgeDataSize = EdgeDataSize;
        Info->EdgeList = PushStruct(&Arena, edge); // Inits list with zeroed stub struct [idx 0].
        Info->EdgeCount++;
        
        StopTiming(&Trace);
        fprintf(stdout, "Init setup: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Get edges line by line.
        
        SetNoDataLine(Info->FirstLine, InspectWidth, NoData, DType);
        for (int Row = 0; Row < Height; Row++)
        {
            GDALDatasetRasterIO(DS, GF_Read, 0, Row, Width, 1, Info->SecondLine+1, Width, 1, DType, 1, 0, 0, LineSweepSize, 0);
            if (!ProcessSweepLine(&Arena, &Info, Row)) return EmptyPoly;
            CopyData(Info->FirstLine, InspectWidth, Info->SecondLine, InspectWidth);
        }
        SetNoDataLine(Info->SecondLine, InspectWidth, NoData, DType);
        if (!ProcessSweepLine(&Arena, &Info, Height-1))  return EmptyPoly;
        
        StopTiming(&Trace);
        fprintf(stdout, "Sweep line: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Sort EdgeList according to connectedness
        
        Info->SortedEdges = PushSize(&Arena, sizeof(u32) * Info->EdgeCount, u32);
        for (usz EdgeIdx = 1; EdgeIdx < Info->EdgeCount; EdgeIdx++)
        {
            Info->SortedEdges[EdgeIdx] = EdgeIdx;
            Info->EdgeList[EdgeIdx].SortedIdx = EdgeIdx;
        }
        
        usz PolyDataSize = Align(RING_DATA_START_SIZE + (Info->EdgeCount * sizeof(v2)), gSysInfo.PageSize);
        if ((Poly.Rings = (ring_info*)GetMemory(PolyDataSize, 0, MEM_WRITE)) == NULL)
        {
            FreeMemory(Arena.Base);
            return EmptyPoly;
        }
        ring_info* Ring = Poly.Rings;
        
        Arena.WriteCur = sizeof(arena_info); // Set this to reuse first part of the arena for tree_node array.
        tree_node* FirstNode = (tree_node*)(Arena.Base + Arena.WriteCur);
        
        StopTiming(&Trace);
        fprintf(stdout, "Pre-loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        u32 FirstIdx = 1;
        bbox2 BBox = BBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
        line_dir PrevDir = LineDir_Right; // Always begins to the right.
        WriteVertexPair(Ring, Affine, Info->EdgeList, &Info->SortedEdges[FirstIdx], &BBox);
        
        for (u32 InsertIdx = 3; InsertIdx < Info->EdgeCount; InsertIdx += 2)
        {
            edge* PrevEdge = &Info->EdgeList[Info->SortedEdges[InsertIdx-1]];
            line_dir NextDir = GetLineDirection(PrevDir, PrevEdge);
            
            u32 NextIdx = (NextDir == LineDir_Down) ? Info->EdgeList[PrevEdge->Below].SortedIdx : Info->EdgeList[PrevEdge->Above].SortedIdx;
            
            if (NextIdx == FirstIdx)
            {
                // Repeat first vertex of ring to close it.
                Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
                Poly.NumRings++;
                Poly.NumVertices += Ring->NumVertices;
                
                tree_node* Node = PushStruct(&Arena, tree_node);
                *Node = {0};
                Node->BBoxArea = (BBox.Max.X - BBox.Min.X) * (BBox.Max.Y - BBox.Min.Y);
                Node->Ring = Ring;
                BBox = BBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
                
                // New ring.
                Ring = GetNextRingInfo(Ring);
                usz RequiredSize = (Info->EdgeCount - InsertIdx + 1) * sizeof(v2) + sizeof(ring_info);
                usz RingOffset = (usz)Ring - (usz)Poly.Rings;
                usz RemainingSize = PolyDataSize - RingOffset;
                
                if (RequiredSize > RemainingSize)
                {
                    // Not supposed to reach here often. Only if raster has a lot of rings.
                    
                    usz NewPolyDataSize = PolyDataSize + gSysInfo.PageSize;
                    void* NewPolyData = GetMemory(NewPolyDataSize, 0, MEM_WRITE);
                    if (!NewPolyData)
                    {
                        FreeMemory(Arena.Base);
                        FreeMemory(Poly.Rings);
                        return EmptyPoly;
                    }
                    CopyData(NewPolyData, NewPolyDataSize, Poly.Rings, PolyDataSize);
                    FreeMemory(Poly.Rings);
                    Poly.Rings = (ring_info*)NewPolyData;
                    PolyDataSize = NewPolyDataSize;
                    Ring = (ring_info*)((u8*)Poly.Rings + RingOffset);
                }
                
                WriteVertexPair(Ring, Affine, Info->EdgeList, &Info->SortedEdges[InsertIdx], &BBox);
                
                FirstIdx = InsertIdx;
                edge* FirstEdge = &Info->EdgeList[Info->SortedEdges[FirstIdx]];
                edge* SecondEdge = &Info->EdgeList[Info->SortedEdges[FirstIdx+1]];
                PrevDir = (SecondEdge->Col > FirstEdge->Col) ? LineDir_Right : LineDir_Left;
            }
            
            else
            {
                u32* Insert = &Info->SortedEdges[InsertIdx];
                u32* Next = &Info->SortedEdges[NextIdx];
                
                edge* InsertEdge = &Info->EdgeList[Insert[0]];
                edge* NextEdge = &Info->EdgeList[Next[0]];
                
                u32 TmpA = Insert[0];
                u32 TmpB = Insert[1];
                
                PrevDir = GetLineDirection(NextDir, NextEdge);
                if (PrevDir == LineDir_Right)
                {
                    Insert[0] = Next[0];
                    Insert[1] = Next[1];
                    Next[0] = TmpA;
                    Next[1] = TmpB;
                    
                    InsertEdge[0].SortedIdx = NextIdx;
                    InsertEdge[1].SortedIdx = NextIdx+1;
                    NextEdge[0].SortedIdx = InsertIdx;
                    NextEdge[1].SortedIdx = InsertIdx+1;
                    
                    WriteVertexPair(Ring, Affine, Info->EdgeList, Insert, &BBox);
                }
                else
                {
                    if (InsertIdx != NextIdx-1)
                    {
                        Insert[0] = Next[0];
                        Insert[1] = Next[-1];
                        Next[0] = TmpB;
                        Next[-1] = TmpA;
                        
                        InsertEdge[0].SortedIdx = NextIdx-1;
                        InsertEdge[1].SortedIdx = NextIdx;
                        NextEdge[0].SortedIdx = InsertIdx;
                        NextEdge[-1].SortedIdx = InsertIdx+1;
                        
                    }
                    else
                    {
                        Insert[0] = TmpB;
                        Insert[1] = TmpA;
                        
                        InsertEdge[0].SortedIdx = InsertIdx+1;
                        InsertEdge[1].SortedIdx = InsertIdx;
                    }
                    
                    WriteVertexPair(Ring, Affine, Info->EdgeList, Insert, &BBox);
                }
            }
        }
        
        Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
        Poly.NumRings++;
        Poly.NumVertices += Ring->NumVertices;
        
        tree_node* Node = PushStruct(&Arena, tree_node);
        *Node = {0};
        Node->BBoxArea = (BBox.Max.X - BBox.Min.X) * (BBox.Max.Y - BBox.Min.Y);
        Node->Ring = Ring;
        
        StopTiming(&Trace);
        fprintf(stdout, "Loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Order polygons by outer/inner.
        
        tree_node NullNode = { DBL_MAX, 0, 0, 0, 0 };
        for (u32 TargetIdx = 1; TargetIdx < Poly.NumRings; TargetIdx++)
        {
            tree_node* TargetNode = &FirstNode[TargetIdx];
            
            tree_node* OuterNode = &NullNode;
            for (u32 TestIdx = 0; TestIdx < Poly.NumRings; TestIdx++)
            {
                tree_node* TestNode = &FirstNode[TestIdx];
                if (TestNode != TargetNode
                    && IsRingInsideRing(TargetNode->Ring, TestNode->Ring)
                    && TestNode->BBoxArea < OuterNode->BBoxArea)
                {
                    OuterNode = TestNode;
                }
                TestNode++;
            }
            
            if (OuterNode != &NullNode)
            {
                TargetNode->Parent = OuterNode;
                if (!OuterNode->Child)
                {
                    OuterNode->Child = TargetNode;
                }
                else
                {
                    tree_node* ChildNode = OuterNode->Child;
                    while (ChildNode->Sibling)
                    {
                        ChildNode = ChildNode->Sibling;
                    }
                    ChildNode->Sibling = TargetNode;
                }
            }
        }
        
        StopTiming(&Trace);
        fprintf(stdout, "Tree creation: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        ring_info NullRing = {0};
        ring_info* PrevRing = &NullRing;
        for (u32 NodeIdx = 0; NodeIdx < Poly.NumRings; NodeIdx++)
        {
            tree_node* Node = &FirstNode[NodeIdx];
            if (Node->Ring->Type == 0)
            {
                PrevRing = PrevRing->Next = Node->Ring;
                if (Node->Child)
                {
                    Node = Node->Child;
                    do
                    {
                        Node->Ring->Type = 1;
                        PrevRing = PrevRing->Next = Node->Ring;
                        Node = Node->Sibling;
                    } while (Node);
                }
            }
        }
        
        StopTiming(&Trace);
        fprintf(stdout, "Tree ordering: %f\n", Trace.Diff);
    }
    else
    {
        // Outline is raster BBox.
        Poly.NumVertices = 5;
        Poly.NumRings = 1;
        
        usz PolyDataSize = sizeof(ring_info) + (5 * sizeof(v2));
        ring_info* Ring = Poly.Rings = (ring_info*)GetMemory(PolyDataSize, 0, MEM_WRITE);
        Ring->NumVertices = 5;
        Ring->Vertices[0] = Ring->Vertices[4] = V2(Affine[0], Affine[3]);
        Ring->Vertices[1] = V2(Affine[0] + (Width * Affine[1]), Affine[3]);
        Ring->Vertices[2] = V2(Affine[0] + (Width * Affine[1]), Affine[3] + (Height * Affine[5]));
        Ring->Vertices[3] = V2(Affine[0], Affine[3] + (Height * Affine[5]));
    }
    
    return Poly;
}

external poly_info
RasterDSToOutline(GDALDatasetH DS)
{
    poly_info Poly = RasterToOutline(DS);
    return Poly;
}

external poly_info
RasterPathToOutline(char* SrcRasterPath)
{
    poly_info Poly = {0};
    
    GDALDatasetH DS = GDALOpen(SrcRasterPath, GA_ReadOnly);
    if (DS)
    {
        Poly = RasterToOutline(DS);
        
        timing Trace;
        StartTiming(&Trace);
        GDALClose(DS);
        StopTiming(&Trace);
        fprintf(stdout, "Closing GDAL: %f\n", Trace.Diff);
    }
    
    return Poly;
}

external void
FreePolyInfo(poly_info Poly)
{
    FreeMemory(Poly.Rings);
}
