#include "tinybase-platform.h"
#include "raster-outline-testblock.cpp"

//================================
// Structs and defines
//================================

#define EDGE_DATA_START_SIZE Kilobyte(64)
#define RING_DATA_START_SIZE Kilobyte(64)

#define Assert(Exp) { if (!(Exp)) *(int*)0 = 0; }

struct edge
{
    edge_type Type;
    int Row;
    int Col;
    u32 Above;
    u32 Below;
    int TimesChecked;
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
    buffer LineSweepMem;
    u8* FirstLine;
    u8* SecondLine;
    u32* ColTable;
    
    u32 InspectWidth;
    u32 BandCount;
    test_block TestBlock;
    double ValueA;
    double ValueB;
    u32 DTypeSize;
    
    buffer EdgeMem;
    u32 EdgeCount;
    u32 VertexCount;
    edge* EdgeList;
    u32* SortedEdges;
    
    ~edge_info()
    {
        FreeMemory(&LineSweepMem);
        FreeMemory(&EdgeMem);
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
SetBleedLine(u8* Line, int Width, double Target, GDALDataType DType)
{
    switch (DType)
    {
        default:
        case GDT_Byte:
        {
            u8 Value = (u8)Target;
            for (u8* Ptr = Line; Ptr < Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_UInt16:
        {
            u16 Value = (u16)Target;
            for (u16* Ptr = (u16*)Line; Ptr < (u16*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Int16:
        {
            i16 Value = (i16)Target;
            for (i16* Ptr = (i16*)Line; Ptr < (i16*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_UInt32:
        {
            u32 Value = (u32)Target;
            for (u32* Ptr = (u32*)Line; Ptr < (u32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Int32:
        {
            i32 Value = (i32)Target;
            for (i32* Ptr = (i32*)Line; Ptr < (i32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Float32:
        {
            f32 Value = (f32)Target;
            for (f32* Ptr = (f32*)Line; Ptr < (f32*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
        
        case GDT_Float64:
        {
            f64 Value = (f64)Target;
            for (f64* Ptr = (f64*)Line; Ptr < (f64*)Line+Width; Ptr++) { *Ptr = Value; }
        } break;
    }
}

internal bool
ProcessSweepLine(edge_info* Info, int Row)
{
    buffer* Mem = &Info->EdgeMem;
    u8* FirstLine = Info->FirstLine;
    u8* SecondLine = Info->SecondLine;
    
    for (int Col = 0; Col < Info->InspectWidth-1; Col++)
    {
        edge_type Type = Info->TestBlock(FirstLine, SecondLine, Info->InspectWidth,
                                         Info->BandCount, Info->ValueA, Info->ValueB);
        if (Type != EdgeType_None)
        {
            Info->EdgeCount++;
            u32 RequiredSize = Info->EdgeCount * sizeof(edge);
            if (Mem->Size < RequiredSize)
            {
                usz NewAllocSize = Max(Mem->Size * 2, RequiredSize);
                buffer NewAlloc = GetMemory(NewAllocSize, 0, MEM_WRITE);
                if (!NewAlloc.Base)
                {
                    return false;
                }
                CopyData(NewAlloc.Base, NewAlloc.Size, Mem->Base, Mem->WriteCur);
                NewAlloc.WriteCur = Mem->WriteCur;
                FreeMemory(Mem);
                
                *Mem = NewAlloc;
                Info->EdgeList = (edge*)NewAlloc.Base;
                Info->EdgeMem = NewAlloc;
            }
            
            edge* NewEdge = PushSize(Mem, sizeof(edge), edge);
            *NewEdge = { Type, Row, Col, 0, 0, false };
            
            // Updates the above and below edge pointers. When BottomLeft or BottomRight,
            // we simply save the new idx. When TopLeft or TopRight, we update the edge
            // pointer at by the ColTable to show that the edge below is the NewEdge, and
            // the NewEdge to show that the above is the previous one at ColTable. This
            // way we can navigate the edge list in any direction. When it's type Cross
            // it behaves like both a Top and Bottom, so it will update the edges first
            // and then update the ColTable with itself.
            
            u32 NewEdgeIdx = Info->EdgeCount-1;
            switch (Type)
            {
                case EdgeType_BottomLeft:
                case EdgeType_BottomRight:
                {
                    Info->ColTable[Col] = NewEdgeIdx;
                    Info->VertexCount++;
                } break;
                
                case EdgeType_TopLeft:
                case EdgeType_TopRight:
                {
                    u32 AboveEdgeIdx = Info->ColTable[Col];
                    edge* AboveEdge = &Info->EdgeList[AboveEdgeIdx];
                    NewEdge->Above = AboveEdgeIdx;
                    AboveEdge->Below = NewEdgeIdx;
                    Info->VertexCount++;
                } break;
                
                case EdgeType_Cross:
                {
                    u32 AboveEdgeIdx = Info->ColTable[Col];
                    edge* AboveEdge = &Info->EdgeList[AboveEdgeIdx];
                    NewEdge->Above = AboveEdgeIdx;
                    AboveEdge->Below = NewEdgeIdx;
                    NewEdge->TimesChecked = -1;
                    
                    Info->ColTable[Col] = NewEdgeIdx;
                    Info->VertexCount += 2; // Crosses count twice.
                } break;
            }
        }
        FirstLine += Info->DTypeSize;
        SecondLine += Info->DTypeSize;
    }
    
    return true;
}

internal void
WriteVertex(buffer* PolyRings, double* Affine, edge* Edge, bbox2* BBox)
{
    v2* Vertex = PushStruct(PolyRings, v2);
    if (!Vertex) Assert(0);
    
    f64 X = Affine[0] + Affine[1] * Edge->Col;
    f64 Y = Affine[3] + Affine[5] * Edge->Row;
    *Vertex = V2(X, Y);
    
    BBox->Min.X = Min(X, BBox->Min.X);
    BBox->Min.Y = Min(Y, BBox->Min.Y);
    BBox->Max.X = Max(X, BBox->Max.X);
    BBox->Max.Y = Max(Y, BBox->Max.Y);
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

internal f64
GetBleedValue(f64 ValueA, f64 ValueB, test_type TestType, GDALDataType DType)
{
    f64 Result = INF64;
    
    f64 MinValue, MaxValue;
    if (DType == GDT_Byte) { MinValue = U8_MIN; MaxValue = U8_MAX; }
    else if (DType == GDT_UInt16) { MinValue = U16_MIN; MaxValue = U16_MAX; }
    else if (DType == GDT_Int16) { MinValue = I16_MIN; MaxValue = I16_MAX; }
    else if (DType == GDT_UInt32) { MinValue = U32_MIN; MaxValue = U32_MAX; }
    else if (DType == GDT_Int32) { MinValue = I32_MIN; MaxValue = I32_MAX; }
    else if (DType == GDT_Float32) { MinValue = F32_MIN; MaxValue = F32_MAX; }
    else if (DType == GDT_Float64) { MinValue = F64_MIN; MaxValue = F64_MAX; }
    
    switch (TestType)
    {
        case TestType_Equal:
        {
            Result = ValueA + 1;
        } break;
        
        case TestType_NotEqual:
        case TestType_BiggerThan:
        case TestType_LessThan:
        case TestType_NotBetween:
        {
            Result = ValueA;
        } break;
        
        case TestType_BiggerOrEqualTo:
        {
            if (ValueA > MinValue)
            {
                union { f64 F; u64 I; } _ValueA = { ValueA };
                _ValueA.I--;
                Result = _ValueA.F;
            }
        } break;
        
        case TestType_LessOrEqualTo:
        {
            if (ValueA < MaxValue)
            {
                union { f64 F; u64 I; } _ValueA = { ValueA };
                _ValueA.I++;
                Result = _ValueA.F;
            }
        } break;
        
        case TestType_Between:
        {
            if (ValueA > MinValue)
            {
                union { f64 F; u64 I; } _ValueA = { ValueA };
                _ValueA.I--;
                Result = _ValueA.F;
            }
            else if (ValueB < MaxValue)
            {
                union { f64 F; u64 I; } _ValueB = { ValueB };
                _ValueB.I++;
                Result = _ValueB.F;
            }
        } break;
    }
    
    return Result;
}

external poly_info
RasterToOutline(GDALDatasetH DS, f64 ValueA, f64 ValueB, test_type TestType,
                int BandCount, int* BandIdx)
{
    poly_info Poly = {0}, EmptyPoly = {0};
    
    GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
    GDALDataType DType = GDALGetRasterDataType(Band);
    int Width = GDALGetRasterXSize(DS);
    int Height = GDALGetRasterYSize(DS);
    int Bands = GDALGetRasterCount(DS);
    double Affine[6];
    GDALGetGeoTransform(DS, Affine);
    
    u32 InspectWidth = Width + 2;   // Two extra columns to protect from overflow.
    u32 InspectHeight = Height + 2; // Two extra rows to protect from overflow.
    
    double BleedValue = GetBleedValue(ValueA, ValueB, TestType, DType);
    if (BleedValue == INF64)
    {
        // Failure to get the bleed value means all pixels would be selected. and
        // getting the BBox amounts to the same thing.
        
        buffer BBox = GetMemory(BBOX_BUFFER_SIZE, 0, MEM_WRITE);
        if (BBox.Base)
        {
            Poly = BBoxOutline(DS, BBox.Base);
        }
        return Poly;
    }
    
    //========================
    // Prepare memory arenas.
    //========================
    
    usz DTypeSize = GetDTypeSize(DType);
    usz ColTableSize = Width * sizeof(u32);
    usz LineSize = InspectWidth * DTypeSize;
    usz LoadPixelsSize = LineSize * 2 * BandCount; // x2 because we load two rows.
    buffer LineSweepMem = GetMemory(LoadPixelsSize + ColTableSize, 0, MEM_WRITE);
    buffer EdgeMem = GetMemory(EDGE_DATA_START_SIZE, 0, MEM_WRITE);
    if (!LineSweepMem.Base || !EdgeMem.Base)
    {
        return EmptyPoly;
    }
    u32 AllBandsLineSize = LineSize * BandCount;
    
    edge_info Info = {0};
    Info.LineSweepMem = LineSweepMem;
    Info.FirstLine = LineSweepMem.Base;
    Info.SecondLine = Info.FirstLine + AllBandsLineSize;
    Info.ColTable = (u32*)(Info.SecondLine + AllBandsLineSize);
    Info.InspectWidth = InspectWidth;
    Info.BandCount = BandCount;
    Info.TestBlock = GetTestBlockCallback(DType, TestType);
    Info.ValueA = ValueA;
    Info.ValueB = ValueB;
    Info.DTypeSize = DTypeSize;
    Info.EdgeMem = EdgeMem;
    Info.EdgeList = PushStruct(&Info.EdgeMem, edge); // Inits list with stub [idx 0].
    Info.EdgeCount++;
    
    //=========================
    // Get edges line by line.
    //=========================
    
    u8* LineReadPtr = Info.SecondLine + DTypeSize;
    SetBleedLine(Info.FirstLine, InspectWidth * BandCount, BleedValue, DType);
    CopyData(Info.SecondLine, AllBandsLineSize, Info.FirstLine, AllBandsLineSize);
    for (int Row = 0; Row < Height; Row++)
    {
        GDALDatasetRasterIO(DS, GF_Read, 0, Row, Width, 1, LineReadPtr, Width, 1, DType,
                            BandCount, BandIdx, 0, LineSize, 0);
        if (!ProcessSweepLine(&Info, Row)) return EmptyPoly;
        
        // This turn's second line is next turn's first line, copy to avoid re-reading.
        CopyData(Info.FirstLine, AllBandsLineSize, Info.SecondLine, AllBandsLineSize);
    }
    
    // Do the last line.
    SetBleedLine(Info.SecondLine, InspectWidth * BandCount, BleedValue, DType);
    if (!ProcessSweepLine(&Info, Height)) return EmptyPoly;
    
    if (Info.EdgeCount == 1) // No occurances found.
    {
        return EmptyPoly;
    }
    
    //========================================
    // Go through edges saving them as rings.
    //========================================
    
    usz RingDataSize = RING_DATA_START_SIZE;
    usz PolyDataSize = Align(RingDataSize + (Info.VertexCount * sizeof(v2)),
                             gSysInfo.PageSize);
    buffer PolyRings = GetMemory(PolyDataSize, 0, MEM_WRITE);
    if (!PolyRings.Base)
    {
        return EmptyPoly;
    }
    Poly.Rings = (ring_info*)PolyRings.Base;
    
    edge* FirstEdge = &Info.EdgeList[1];
    edge* EndOfEdgeList = &Info.EdgeList[Info.EdgeCount];
    while (FirstEdge < EndOfEdgeList)
    {
        usz NumVerticesLeft = Info.VertexCount - Poly.NumVertices;
        usz RequiredRingSize = (sizeof(ring_info)
                                + (NumVerticesLeft * sizeof(v2))
                                + sizeof(tree_node));
        if (RequiredRingSize > (PolyRings.Size - PolyRings.WriteCur))
        {
            RingDataSize += RING_DATA_START_SIZE;
            usz NewSize = Align(RingDataSize + (Info.VertexCount * sizeof(v2)),
                                gSysInfo.PageSize);
            buffer NewMem = GetMemory(NewSize, 0, MEM_WRITE);
            if (!NewMem.Base)
            {
                return EmptyPoly;
            }
            CopyData(NewMem.Base, NewMem.Size, PolyRings.Base, PolyRings.WriteCur);
            NewMem.WriteCur = PolyRings.WriteCur;
            FreeMemory(&PolyRings);
            PolyRings = NewMem;
        }
        
        ring_info* Ring = PushStruct(&PolyRings, ring_info);
        if (!Ring) Assert(0);
        
        bbox2 BBox = BBox2(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
        edge* Edge = FirstEdge;
        line_dir Dir = LineDir_Right; // Going clockwise always starts to the right.
        
        do
        {
            WriteVertex(&PolyRings, Affine, Edge, &BBox);
            Edge->TimesChecked++;
            
            switch (Dir)
            {
                case LineDir_Right:
                {
                    Edge++;
                    if (Edge->Type == EdgeType_BottomLeft) Dir = LineDir_Down;
                    else if (Edge->Type == EdgeType_TopLeft) Dir = LineDir_Up;
                    else if (Edge->Type == EdgeType_Cross) Dir = LineDir_Down;
                    else Assert(0);
                } break;
                
                case LineDir_Left:
                {
                    Edge--;
                    if (Edge->Type == EdgeType_BottomRight) Dir = LineDir_Down;
                    else if (Edge->Type == EdgeType_TopRight) Dir = LineDir_Up;
                    else if (Edge->Type == EdgeType_Cross) Dir = LineDir_Up;
                    else Assert(0);
                } break;
                
                case LineDir_Down:
                {
                    Edge = &Info.EdgeList[Edge->Below];
                    if (Edge->Type == EdgeType_TopLeft) Dir = LineDir_Left;
                    else if (Edge->Type == EdgeType_TopRight) Dir = LineDir_Right;
                    else if (Edge->Type == EdgeType_Cross) Dir = LineDir_Left;
                    else Assert(0);
                } break;
                
                case LineDir_Up:
                {
                    Edge = &Info.EdgeList[Edge->Above];
                    if (Edge->Type == EdgeType_BottomLeft) Dir = LineDir_Left;
                    else if (Edge->Type == EdgeType_BottomRight) Dir = LineDir_Right;
                    else if (Edge->Type == EdgeType_Cross) Dir = LineDir_Right;
                    else Assert(0);
                } break;
            }
        } while (Edge != FirstEdge);
        
        // Repeat the first edge to close the polygon.
        WriteVertex(&PolyRings, Affine, Edge, &BBox);
        Info.VertexCount++;
        
        Ring->NumVertices = (v2*)&PolyRings.Base[PolyRings.WriteCur] - Ring->Vertices;
        Poly.NumVertices += Ring->NumVertices;
        Poly.NumRings++;
        
        tree_node* Node = PushStruct(&PolyRings, tree_node);
        if (!Node) Assert(0);
        
        Node->BBoxArea = (BBox.Max.X - BBox.Min.X) * (BBox.Max.Y - BBox.Min.Y);
        Node->RingOffset = (usz)Node - (usz)Ring;
        
        while (FirstEdge < EndOfEdgeList)
        {
            FirstEdge++;
            if (FirstEdge->TimesChecked == 0)
            {
                break;
            }
        }
    }
    
    Poly.Rings = (ring_info*)PolyRings.Base; // Updates in case resizing was necessary.
    
    //================================
    // Order polygons by outer/inner.
    //================================
    
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
    tree_node* Node = FirstNode;
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
