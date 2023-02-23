#include "gdal.h"
#include "geotypes-base.h"
#include "tinybase-platform.h"

#include "raster-outline.h"

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
ProcessSweepLine(edge_data* Edges, int Row)
{
    for (int Col = 0; Col < Edges->LineSweepSize-1; Col++)
    {
        edge_type Type = Edges->TestBlock(Edges->FirstLine + Col, Edges->SecondLine + Col, Edges->NoData);
        if (Type != EdgeType_None)
        {
            u32 NewEdgeIdx = Edges->Count++;
            usz RequiredSize = (Edges->Count * sizeof(edge)) + (Edges->Count * sizeof(u32));
            usz CurrentSize = Edges->Arena.Size - Edges->ListOffset;
            
            if (RequiredSize > CurrentSize)
            {
                usz NewAllocSize = Edges->ListOffset + (RequiredSize * 2);
                u8* NewAlloc = (u8*)GetMemory(NewAllocSize, 0, MEM_WRITE);
                if (!NewAlloc)
                {
                    return false;
                }
                CopyData(NewAlloc, NewAllocSize, Edges->Arena.Base, Edges->Arena.WriteCur);
                FreeMemory(Edges->Arena.Base);
                
                Edges->Arena.Base = NewAlloc;
                Edges->Arena.Size = NewAllocSize;
                Edges->FirstLine = Edges->Arena.Base + Edges->ColHashTableSize;
                Edges->SecondLine = Edges->FirstLine + Edges->LineSweepSize;
                Edges->List = (edge*)(Edges->SecondLine + Edges->LineSweepSize);
            }
            
            edge* NewEdge = PushStruct(&Edges->Arena, edge);
            *NewEdge = { Type, Row, Col, 0, 0 };
            
            u32* ColHashTable = (u32*)Edges->Arena.Base;
            u32 AboveEdgeIdx = ColHashTable[Col];
            if (AboveEdgeIdx > 0)
            {
                edge* AboveEdge = Edges->List + AboveEdgeIdx;
                NewEdge->Above = AboveEdgeIdx;
                AboveEdge->Below = NewEdgeIdx;
            }
            ColHashTable[Col] = NewEdgeIdx;
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
WriteVertexPair(ring_info* Ring, double* Affine, edge* EdgeList, u32* EdgeIdx)
{
    edge* FirstEdge = &EdgeList[EdgeIdx[0]];
    edge* SecondEdge = &EdgeList[EdgeIdx[1]];
    
    f64 X = Affine[0] + Affine[1] * FirstEdge->Col;
    f64 Y = Affine[3] + Affine[5] * FirstEdge->Row;
    Ring->Vertices[Ring->NumVertices++] = V2(X, Y);
    
    X = Affine[0] + Affine[1] * SecondEdge->Col;
    Y = Affine[3] + Affine[5] * SecondEdge->Row;
    Ring->Vertices[Ring->NumVertices++]= V2(X, Y);
}

internal ring_info*
GetNextRingInfo(ring_info* Ring)
{
    ring_info* Result = (ring_info*)(((v2*)&Ring[1]) + Ring->NumVertices);
    return Result;
}

internal bool
IsRingInsideRing(ring_info* A, ring_info* B)
{
    if (A != B
        && B->Type == 0) // This is only until the code supports rings inside inner rings.
    {
        v2* Test = A->Vertices;
        usz RayRightCount = 0, RayUpCount = 0, RayDownCount = 0;
        
        v2* BP0, *BP1, *BPLast = &B->Vertices[B->NumVertices];
        BP0 = BP1 = B->Vertices;
        while (++BP1 < BPLast)
        {
            if (BP0->X == BP1->X) // Vertical line
            {
                f64 YMin = Min(BP0->Y, BP1->Y);
                f64 YMax = Max(BP0->Y, BP1->Y);
                
                if ((Test->X < BP0->X) && (Test->Y >= YMin) && (Test->Y <= YMax)) RayRightCount++;
                if ((Test->X == BP0->X) && (Test->Y < YMin)) RayUpCount++;
                if ((Test->X == BP0->X) && (Test->Y > YMax)) RayDownCount++;
            }
            else // Horizontal line
            {
                f64 XMin = Min(BP0->X, BP1->X);
                f64 XMax = Max(BP0->X, BP1->X);
                
                if ((Test->X >= XMin) && (Test->X <= XMax) && (Test->Y < BP0->Y)) RayUpCount++;
                if ((Test->X >= XMin) && (Test->X <= XMax) && (Test->Y > BP0->Y)) RayDownCount++;
                if ((Test->Y == BP0->Y) && (Test->X < XMin)) RayRightCount++;
            }
            
            BP0++;
        }
        
        return (RayRightCount % 0) && (RayUpCount % 0) && (RayDownCount % 0);
    }
    
    return false;
}

internal poly_info
RasterToOutline(GDALDatasetH DS)
{
    poly_info Poly = {0};
    
    timing Trace;
    StartTiming(&Trace);
    
    GDALRasterBandH Band = GDALGetRasterBand(DS, 1);
    GDALDataType DType = GDALGetRasterDataType(Band);
    int Width = GDALGetRasterXSize(DS);
    int Height = GDALGetRasterYSize(DS);
    int Bands = GDALGetRasterCount(DS);
    
    double Affine[6];
    GDALGetGeoTransform(DS, Affine);
    
    int RasterHasNoData = 0;
    double NoData = GDALGetRasterNoDataValue(Band, &RasterHasNoData); // Check if DType isn't i64 or u64.
    
    if (RasterHasNoData)
    {
        int InspectWidth = Width + 2; // One extra pixel on the left and right, to guard from overflow.
        int InspectHeight = Height + 2; // One extra row on top and bottom, to guard from overflow.
        usz DTypeSize = GetDTypeSize(DType);
        
        edge_data Edges = {0};
        Edges.ColHashTableSize = Width * sizeof(u32);
        Edges.LineSweepSize = InspectWidth * DTypeSize;
        Edges.ListOffset = Edges.ColHashTableSize + (Edges.LineSweepSize * 2);
        
        usz EdgeDataSize = Edges.ListOffset + EDGE_DATA_START_SIZE;
        u8* EdgeDataMem = (u8*)GetMemory(EdgeDataSize, 0, MEM_WRITE);
        if (!EdgeDataMem)
        {
            return Poly;
        }
        
        Edges.Arena = Buffer(EdgeDataMem, 0, Align(EdgeDataSize, gSysInfo.PageSize));
        Edges.TestBlock = GetTestBlockCallback(DType);
        Edges.NoData = NoData;
        
        PushSize(&Edges.Arena, Edges.ColHashTableSize, u32);
        Edges.FirstLine = PushSize(&Edges.Arena, Edges.LineSweepSize, u8);
        Edges.SecondLine = PushSize(&Edges.Arena, Edges.LineSweepSize, u8);
        Edges.List = PushStruct(&Edges.Arena, edge); // Inits list with zeroed stub struct [idx 0].
        Edges.Count++;
        
        StopTiming(&Trace);
        fprintf(stdout, "Init setup: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Get edges line by line.
        
        SetNoDataLine(Edges.FirstLine, InspectWidth, NoData, DType);
        for (int Row = 0; Row < Height; Row++)
        {
            GDALDatasetRasterIO(DS, GF_Read, 0, Row, Width, 1, Edges.SecondLine+1, Width, 1, DType, 1, 0, 0, Edges.LineSweepSize, 0);
            ProcessSweepLine(&Edges, Row);
            CopyData(Edges.FirstLine, InspectWidth, Edges.SecondLine, InspectWidth);
        }
        SetNoDataLine(Edges.SecondLine, InspectWidth, NoData, DType);
        ProcessSweepLine(&Edges, Height-1);
        
        StopTiming(&Trace);
        fprintf(stdout, "Sweep line: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Sort EdgeList according to connectedness
        
        u32* SortedEdges = (u32*)(Edges.List + Edges.Count);
        for (usz EdgeIdx = 1; EdgeIdx < Edges.Count; EdgeIdx++)
        {
            SortedEdges[EdgeIdx] = EdgeIdx;
            Edges.List[EdgeIdx].SortedIdx = EdgeIdx;
        }
        
        usz PolyDataSize = Align(RING_DATA_START_SIZE + (Edges.Count * sizeof(v2)), gSysInfo.PageSize);
        void* PolyData = GetMemory(PolyDataSize, 0, MEM_WRITE);
        if (!PolyData)
        {
            return Poly;
        }
        ring_info* Ring = Poly.Rings = (ring_info*)PolyData;
        
        u32 FirstIdx = 1;
        WriteVertexPair(Ring, Affine, Edges.List, &SortedEdges[FirstIdx]);
        line_dir PrevDir = LineDir_Right; // Always begins to the right.
        
        StopTiming(&Trace);
        fprintf(stdout, "Pre-loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        for (u32 InsertIdx = 3; InsertIdx < Edges.Count; InsertIdx += 2)
        {
            edge* PrevEdge = &Edges.List[SortedEdges[InsertIdx-1]];
            line_dir NextDir = GetLineDirection(PrevDir, PrevEdge);
            
            u32 NextIdx = (NextDir == LineDir_Down) ? Edges.List[PrevEdge->Below].SortedIdx : Edges.List[PrevEdge->Above].SortedIdx;
            
            if (NextIdx == FirstIdx)
            {
                // Repeat first vertex of ring to close it.
                Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
                Poly.NumRings++;
                Poly.NumVertices += Ring->NumVertices;
                
                // New ring.
                Ring = GetNextRingInfo(Ring);
                usz RequiredSize = (Edges.Count - InsertIdx + 1) * sizeof(v2) + sizeof(ring_info);
                usz RingOffset = (usz)Ring - (usz)Poly.Rings;
                usz RemainingSize = PolyDataSize - RingOffset;
                
                if (RequiredSize > RemainingSize)
                {
                    // Not supposed to reach here often. Only if raster has a lot of rings.
                    
                    usz NewPolyDataSize = PolyDataSize + gSysInfo.PageSize;
                    void* NewPolyData = GetMemory(NewPolyDataSize, 0, MEM_WRITE);
                    if (!NewPolyData)
                    {
                        return Poly;
                    }
                    CopyData(NewPolyData, NewPolyDataSize, Poly.Rings, PolyDataSize);
                    FreeMemory(Poly.Rings);
                    Poly.Rings = (ring_info*)NewPolyData;
                    PolyDataSize = NewPolyDataSize;
                    Ring = (ring_info*)((u8*)Poly.Rings + RingOffset);
                }
                
                WriteVertexPair(Ring, Affine, Edges.List, &SortedEdges[InsertIdx]);
                
                FirstIdx = InsertIdx;
                edge* FirstEdge = &Edges.List[SortedEdges[FirstIdx]];
                edge* SecondEdge = &Edges.List[SortedEdges[FirstIdx+1]];
                PrevDir = (SecondEdge->Col > FirstEdge->Col) ? LineDir_Right : LineDir_Left;
            }
            
            else
            {
                u32* Insert = &SortedEdges[InsertIdx];
                u32* Next = &SortedEdges[NextIdx];
                
                edge* InsertEdge = &Edges.List[Insert[0]];
                edge* NextEdge = &Edges.List[Next[0]];
                
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
                    
                    WriteVertexPair(Ring, Affine, Edges.List, Insert);
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
                    
                    WriteVertexPair(Ring, Affine, Edges.List, Insert);
                }
            }
        }
        
        StopTiming(&Trace);
        fprintf(stdout, "Loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Order polygons by outer/inner.
        
        Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
        Poly.NumRings++;
        Poly.NumVertices += Ring->NumVertices;
        
        ring_info* PrevRing = Poly.Rings; // First ring is always outer.
        for (u32 Count = 1; Count < Poly.NumRings; Count++)
        {
            ring_info* ThisRing = GetNextRingInfo(PrevRing);
#if 0
            ring_info* VerifyRing = Poly.Rings[0];
            for (u32 Count2 = 0; Count2 < Poly.NumRings; Count2++)
            {
                VerifyRing = GetNextRingInfo(VerifyRing);
            }
#else
            PrevRing->Next = ThisRing;
            PrevRing = ThisRing;
#endif
        }
        
        FreeMemory(Edges.Arena.Base);
        
        StopTiming(&Trace);
        fprintf(stdout, "Post-loop: %f\n", Trace.Diff);
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
