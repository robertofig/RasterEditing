#include "tinybase-platform.h"
#include "raster-outline-testblock.cpp"

//================================
// Structs and defines
//================================

#define EDGE_DATA_START_SIZE Kilobyte(64)
#define RING_DATA_START_SIZE Kilobyte(64)

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

struct edge_info
{
    u8* FirstLine;
    u8* SecondLine;
    u32* ColHashTable;
    
    u32 InspectWidth;
    u32 BandCount;
    test_block TestBlock;
    double Value;
    u32 DTypeSize;
    
    buffer EdgeArena;
    u32 EdgeCount;
    edge* EdgeList;
    u32* SortedEdges;
    
    ~edge_info()
    {
        FreeMemory(FirstLine);
        FreeMemory(EdgeArena.Base);
    }
};

struct tree_node
{
    f64 BBoxArea;
    u32 RingOffset;
    
    tree_node* Parent;
    tree_node* Sibling;
    tree_node* Child;
};

//================================
// Functions
//================================

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

internal u32
RegisterEdge(edge_info* Info, buffer* Arena, int Row, int Col, edge_type Type)
{
    edge* NewEdge = PushStruct(Arena, edge);
    *NewEdge = { Type, Row, Col, 0, 0 };
    return Info->EdgeCount++;
}

internal void
UpdateAboveBelowRef(edge_info* Info, int Col, edge* NewEdge, u32 NewEdgeIdx)
{
    u32 AboveEdgeIdx = Info->ColHashTable[Col];
    if (AboveEdgeIdx > 0)
    {
        edge* AboveEdge = &Info->EdgeList[AboveEdgeIdx];
        NewEdge->Above = AboveEdgeIdx;
        AboveEdge->Below = NewEdgeIdx;
    }
}

internal bool
ProcessSweepLine(edge_info* Info, int Row)
{
    buffer* Arena = &Info->EdgeArena;
    u8* FirstLine = Info->FirstLine;
    u8* SecondLine = Info->SecondLine;
    for (int Col = 0; Col < Info->InspectWidth-1; Col++)
    {
        edge_type Type = Info->TestBlock(FirstLine, SecondLine, Info->InspectWidth, Info->BandCount, Info->Value);
        if (Type != EdgeType_None)
        {
            Info->EdgeCount += (Type >= EdgeType_TopLeftBottomRight) ? 2 : 1;
            u32 RequiredSize = Info->EdgeCount * (sizeof(edge)+sizeof(u32)); // u32 is for sorted index.
            if (Arena->Size < RequiredSize)
            {
                usz NewAllocSize = Max(Arena->Size * 2, RequiredSize);
                u8* NewAlloc = (u8*)GetMemory(NewAllocSize, 0, MEM_WRITE);
                if (!NewAlloc)
                {
                    return false;
                }
                CopyData(NewAlloc, Arena->WriteCur, Arena->Base, Arena->WriteCur);
                FreeMemory(Arena->Base);
                
                Arena->Base = NewAlloc;
                Arena->Size = Align(NewAllocSize, gSysInfo.PageSize);
                Info->EdgeList = (edge*)NewAlloc;
            }
            
            if (Type == EdgeType_TopLeftBottomRight)
            {
                edge* NewEdges = PushSize(Arena, sizeof(edge)*2, edge);
                NewEdges[0] = { EdgeType_TopLeft, Row, Col, 0, 0 };
                NewEdges[1] = { EdgeType_BottomRight, Row, Col, 0, 0 };
                UpdateAboveBelowRef(Info, Col, NewEdges, Info->EdgeCount-2);
                Info->ColHashTable[Col] = Info->EdgeCount-1;
            }
            else if (Type == EdgeType_TopRightBottomLeft)
            {
                edge* NewEdges = PushSize(Arena, sizeof(edge)*2, edge);
                NewEdges[0] = { EdgeType_BottomLeft, Row, Col, 0, 0 };
                NewEdges[1] = { EdgeType_TopRight, Row, Col, 0, 0 };
                UpdateAboveBelowRef(Info, Col, NewEdges+1, Info->EdgeCount-1);
                Info->ColHashTable[Col] = Info->EdgeCount-2;
            }
            else
            {
                edge* NewEdges = PushSize(Arena, sizeof(edge), edge);
                NewEdges[0] = { Type, Row, Col, 0, 0 };
                UpdateAboveBelowRef(Info, Col, NewEdges, Info->EdgeCount-1);
                Info->ColHashTable[Col] = Info->EdgeCount-1;
            }
        }
        FirstLine += Info->DTypeSize;
        SecondLine += Info->DTypeSize;
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
GetRingFrom(tree_node* Node)
{
    ring_info* Result = (ring_info*)((u8*)Node - Node->RingOffset);
    return Result;
}

internal tree_node*
GetNextTreeNode(tree_node* Node)
{
    ring_info* NextRing = (ring_info*)&Node[1];
    tree_node* Result = (tree_node*)&NextRing->Vertices[NextRing->NumVertices];
    return Result;
}

internal bool
CheckCollision(v2 Test, v2 P0, v2 P1, v2 P2, v2 P3)
{
    if (P1.X == P2.X) // Vertical line
    {
        f64 YMin = Min(P1.Y, P2.Y);
        f64 YMax = Max(P1.Y, P2.Y);
        return (Test.X < P1.X && Test.Y >= YMin && Test.Y <= YMax);
    }
    else if (P1.Y == Test.Y) // Collinear Horizontal line
    {
        f64 XMin = Min(P1.X, P2.X);
        if (Test.X < XMin)  // Hits ray to the right.
        {
            return ((P0.Y < P1.Y && P2.Y < P3.Y) || (P0.Y > P1.Y && P2.Y > P3.Y));
        }
    }
    return false;
}

internal bool
IsRingInsideRing(ring_info* A, ring_info* B)
{
    if (A != B)
    {
        v2 Test = (A->Vertices[0].X < A->Vertices[1].X) ? A->Vertices[0] : (A->Vertices[1].X < A->Vertices[2].X) ? A->Vertices[1] : A->Vertices[2];
        usz RayCount = 0;
        
        // BP1-BP2 is the test line, BP0-BP1 is the line before and BP2-BP3 is the line after.
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

external poly_info
RasterToOutline(GDALDatasetH DS, double Value, test_type TestType, int BandCount, int* BandIdx)
{
    poly_info Poly = {0}, EmptyPoly = {0};
    
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
    double NoData = GDALGetRasterNoDataValue(Band, &RasterHasNoData); // TODO: Check if DType isn't i64 or u64.
    if (!RasterHasNoData)
    {
        NoData = (Value == 0) ? 1 : 0;
    }
    
    // Prepare memory arena.
    
    u32 ColHashTableSize = Width * sizeof(u32);
    u32 LineSweepSize = InspectWidth * DTypeSize;
    void* LineSweepMem = GetMemory(LineSweepSize*2*BandCount + ColHashTableSize, 0, MEM_WRITE);
    void* EdgeArenaMem = GetMemory(EDGE_DATA_START_SIZE, 0, MEM_WRITE);
    if (!LineSweepMem || !EdgeArenaMem)
    {
        return EmptyPoly;
    }
    u32 AllBandsLineSize = LineSweepSize * BandCount;
    
    edge_info Info = {0};
    Info.FirstLine = (u8*)LineSweepMem;
    Info.SecondLine = Info.FirstLine + AllBandsLineSize;
    Info.ColHashTable = (u32*)(Info.SecondLine + AllBandsLineSize);
    Info.InspectWidth = InspectWidth;
    Info.BandCount = BandCount;
    Info.TestBlock = GetTestBlockCallback(DType, TestType);
    Info.Value = Value;
    Info.DTypeSize = DTypeSize;
    Info.EdgeArena = Buffer(EdgeArenaMem, 0, Align(EDGE_DATA_START_SIZE, gSysInfo.PageSize));
    Info.EdgeList = PushStruct(&Info.EdgeArena, edge); // Inits list with zeroed stub struct [idx 0].
    Info.EdgeCount++;
    
    // Get edges line by line.
    
    u8* LineReadPtr = Info.SecondLine + DTypeSize;
    SetNoDataLine(Info.FirstLine, InspectWidth * BandCount, NoData, DType);
    CopyData(Info.SecondLine, AllBandsLineSize, Info.FirstLine, AllBandsLineSize);
    for (int Row = 0; Row < Height; Row++)
    {
        GDALDatasetRasterIO(DS, GF_Read, 0, Row, Width, 1, LineReadPtr, Width, 1, DType, BandCount, BandIdx, 0, LineSweepSize, 0);
        if (!ProcessSweepLine(&Info, Row)) return EmptyPoly;
        CopyData(Info.FirstLine, AllBandsLineSize, Info.SecondLine, AllBandsLineSize);
    }
    SetNoDataLine(Info.SecondLine, InspectWidth * BandCount, NoData, DType);
    if (!ProcessSweepLine(&Info, Height)) return EmptyPoly;
    
    if (Info.EdgeCount == 1) // No occurances found.
    {
        return EmptyPoly;
    }
    
    // Sort EdgeList according to connectedness
    
    Info.SortedEdges = PushSize(&Info.EdgeArena, sizeof(u32) * Info.EdgeCount, u32);
    for (usz EdgeIdx = 1; EdgeIdx < Info.EdgeCount; EdgeIdx++)
    {
        Info.SortedEdges[EdgeIdx] = EdgeIdx;
        Info.EdgeList[EdgeIdx].SortedIdx = EdgeIdx;
    }
    
    usz PolyDataSize = Align(RING_DATA_START_SIZE + (Info.EdgeCount * sizeof(v2)), gSysInfo.PageSize);
    if ((Poly.Rings = (ring_info*)GetMemory(PolyDataSize, 0, MEM_WRITE)) == NULL)
    {
        return EmptyPoly;
    }
    ring_info* Ring = Poly.Rings;
    
    u32 FirstIdx = 1;
    bbox2 BBox = BBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
    line_dir PrevDir = LineDir_Right; // Always begins to the right.
    WriteVertexPair(Ring, Affine, Info.EdgeList, &Info.SortedEdges[FirstIdx], &BBox);
    
    for (u32 InsertIdx = 3; InsertIdx < Info.EdgeCount; InsertIdx += 2)
    {
        if (InsertIdx == 4801)
        {
            int Z = 0;
        }
        
        edge* PrevEdge = &Info.EdgeList[Info.SortedEdges[InsertIdx-1]];
        line_dir NextDir = GetLineDirection(PrevDir, PrevEdge);
        
        u32 NextIdx = (NextDir == LineDir_Down) ? Info.EdgeList[PrevEdge->Below].SortedIdx : Info.EdgeList[PrevEdge->Above].SortedIdx;
        
        if (NextIdx == FirstIdx)
        {
            // Repeat first vertex of ring to close it.
            Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
            Poly.NumRings++;
            Poly.NumVertices += Ring->NumVertices;
            
            tree_node* Node = (tree_node*)&Ring->Vertices[Ring->NumVertices];
            Node->BBoxArea = (BBox.Max.X - BBox.Min.X) * (BBox.Max.Y - BBox.Min.Y);
            Node->RingOffset = (usz)Node - (usz)Ring;
            BBox = BBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
            
            // New ring.
            Ring = (ring_info*)&Node[1];
            usz RequiredSize = (Info.EdgeCount - InsertIdx + 1) * sizeof(v2) + sizeof(ring_info) + sizeof(tree_node);
            usz RingOffset = (usz)Ring - (usz)Poly.Rings;
            usz RemainingSize = PolyDataSize - RingOffset;
            
            if (RequiredSize > RemainingSize)
            {
                // Not supposed to reach here often. Only if raster has a lot of rings.
                
                usz NewPolyDataSize = PolyDataSize + gSysInfo.PageSize;
                void* NewPolyData = GetMemory(NewPolyDataSize, 0, MEM_WRITE);
                if (!NewPolyData)
                {
                    FreeMemory(Poly.Rings);
                    return EmptyPoly;
                }
                CopyData(NewPolyData, NewPolyDataSize, Poly.Rings, PolyDataSize);
                FreeMemory(Poly.Rings);
                Poly.Rings = (ring_info*)NewPolyData;
                PolyDataSize = NewPolyDataSize;
                Ring = (ring_info*)((u8*)Poly.Rings + RingOffset);
            }
            
            WriteVertexPair(Ring, Affine, Info.EdgeList, &Info.SortedEdges[InsertIdx], &BBox);
            
            FirstIdx = InsertIdx;
            edge* FirstEdge = &Info.EdgeList[Info.SortedEdges[FirstIdx]];
            edge* SecondEdge = &Info.EdgeList[Info.SortedEdges[FirstIdx+1]];
            PrevDir = (SecondEdge->Col > FirstEdge->Col) ? LineDir_Right : LineDir_Left;
        }
        
        else
        {
            u32* Insert = &Info.SortedEdges[InsertIdx];
            u32* Next = &Info.SortedEdges[NextIdx];
            
            edge* InsertEdge = &Info.EdgeList[Insert[0]];
            edge* NextEdge = &Info.EdgeList[Next[0]];
            
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
                
                WriteVertexPair(Ring, Affine, Info.EdgeList, Insert, &BBox);
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
                
                WriteVertexPair(Ring, Affine, Info.EdgeList, Insert, &BBox);
            }
        }
    }
    
    Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
    Poly.NumRings++;
    Poly.NumVertices += Ring->NumVertices;
    
    tree_node* Node = (tree_node*)&Ring->Vertices[Ring->NumVertices];
    Node->BBoxArea = (BBox.Max.X - BBox.Min.X) * (BBox.Max.Y - BBox.Min.Y);
    Node->RingOffset = (usz)Node - (usz)Ring;
    
    // Order polygons by outer/inner.
    
    tree_node NullNode = { DBL_MAX, 0, 0, 0, 0 };
    tree_node* FirstNode = (tree_node*)&Poly.Rings->Vertices[Poly.Rings->NumVertices];
    tree_node* TargetNode = GetNextTreeNode(FirstNode);
    for (u32 TargetIdx = 1; TargetIdx < Poly.NumRings; TargetIdx++)
    {
        tree_node* OuterNode = &NullNode;
        tree_node* TestNode = FirstNode;
        ring_info* TargetRing = GetRingFrom(TargetNode);
        for (u32 TestIdx = 0; TestIdx < Poly.NumRings; TestIdx++)
        {
            if (TestNode != TargetNode
                && IsRingInsideRing(TargetRing, GetRingFrom(TestNode))
                && TestNode->BBoxArea < OuterNode->BBoxArea)
            {
                TargetRing->Type++;
                OuterNode = TestNode;
            }
            TestNode = GetNextTreeNode(TestNode);
        }
        
        TargetRing->Type &= 0x1; // Forces type to be 0 or 1.
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
        
        TargetNode = GetNextTreeNode(TargetNode);
    }
    
    ring_info NullRing = {0};
    ring_info* PrevRing = &NullRing;
    Node = FirstNode;
    for (u32 NodeIdx = 0; NodeIdx < Poly.NumRings; NodeIdx++)
    {
        ring_info* Ring = GetRingFrom(Node);
        if (Ring->Type  == 0)
        {
            PrevRing = PrevRing->Next = Ring;
            if (Node->Child)
            {
                tree_node* ChildNode = Node->Child;
                do
                {
                    ring_info* ChildRing = GetRingFrom(ChildNode);
                    PrevRing = PrevRing->Next = ChildRing;
                    ChildNode= ChildNode->Sibling;
                } while (ChildNode);
            }
        }
        Node = GetNextTreeNode(Node);
    }
    
    return Poly;
}

external poly_info
BBoxOutline(GDALDatasetH DS, u8* BBoxBuffer)
{
    poly_info Poly = {0};
    
    // Outline is raster BBox.
    Poly.NumVertices = 5;
    Poly.NumRings = 1;
    
    int Width = GDALGetRasterXSize(DS);
    int Height = GDALGetRasterYSize(DS);
    double Affine[6];
    GDALGetGeoTransform(DS, Affine);
    
    usz PolyDataSize = BBOX_BUFFER_SIZE;
    Poly.Rings = (ring_info*)BBoxBuffer;
    Poly.Rings->NumVertices = 5;
    Poly.Rings->Vertices[0] = Poly.Rings->Vertices[4] = V2(Affine[0], Affine[3]);
    Poly.Rings->Vertices[1] = V2(Affine[0] + (Width * Affine[1]), Affine[3]);
    Poly.Rings->Vertices[2] = V2(Affine[0] + (Width * Affine[1]), Affine[3] + (Height * Affine[5]));
    Poly.Rings->Vertices[3] = V2(Affine[0], Affine[3] + (Height * Affine[5]));
    
    return Poly;
}

external void
FreePolyInfo(poly_info Poly)
{
    if (Poly.Rings)
    {
        FreeMemory(Poly.Rings);
    }
}
