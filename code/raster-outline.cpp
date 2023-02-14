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
ProcessSweepLine(sweep_line Line, int Row, buffer* EdgeData, u32* ColHashTable)
{
    for (int Col = 0; Col < Line.Width-1; Col++)
    {
        edge_type Type = Line.TestBlock(Line.First + Col, Line.Second + Col, Line.NoData);
        if (Type != EdgeType_None)
        {
            u32 NewEdgeIdx = (EdgeData->WriteCur + 1) / sizeof(edge);
            usz SortedEdgesSize = NewEdgeIdx * sizeof(u32);
            usz RequiredSize = EdgeData->WriteCur + sizeof(edge) + SortedEdgesSize;
            
            if (RequiredSize > EdgeData->Size)
            {
                usz NewAllocSize = RequiredSize * 2;
                u8* NewAlloc = (u8*)GetMemory(NewAllocSize, 0, MEM_WRITE);
                if (!NewAlloc)
                {
                    return false;
                }
                CopyData(NewAlloc, NewAllocSize, EdgeData->Base, EdgeData->WriteCur);
                FreeMemory(EdgeData->Base);
                
                EdgeData->Base = NewAlloc;
                EdgeData->Size = NewAllocSize;
            }
            
            edge* NewEdge = PushStruct(EdgeData, edge);
            *NewEdge = { Type, Row, Col, 0, 0 };
            
            i32 AboveEdgeIdx = ColHashTable[Col];
            if (AboveEdgeIdx > 0)
            {
                edge* AboveEdge = (edge*)EdgeData->Base + AboveEdgeIdx;
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

external poly_info
RasterToOutline(char* SrcRasterPath)
{
    poly_info Poly = {0};
    
    timing Trace;
    StartTiming(&Trace);
    
    GDALDatasetH DS = GDALOpen(SrcRasterPath, GA_ReadOnly);
    if (!DS)
    {
        return Poly;
    }
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
        test_block TestBlock = GetTestBlockCallback(DType);
        
        usz ColHashTableSize = Width * sizeof(u32);
        usz LineSweepSize = InspectWidth * DTypeSize * 2;
        usz LineDataSize = ColHashTableSize + LineSweepSize;
        
        u8* LineDataMem = (u8*)GetMemory(LineDataSize, 0, MEM_WRITE);
        u8* EdgeDataMem = (u8*)GetMemory(EDGE_DATA_START_SIZE, 0, MEM_WRITE);
        if (!(LineDataMem && EdgeDataMem))
        {
            return Poly;
        }
        
        u32* ColHashTable = (u32*)LineDataMem;
        u8* FirstLine = LineDataMem + ColHashTableSize;
        u8* SecondLine = FirstLine + (InspectWidth * DTypeSize);
        
        buffer EdgeData = Buffer(EdgeDataMem, 0, Align(EDGE_DATA_START_SIZE, gSysInfo.PageSize));
        edge* NullEdge = PushStruct(&EdgeData, edge); // Zeroed default struct.
        
        StopTiming(&Trace);
        fprintf(stdout, "Init setup: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Get edges line by line.
        
        sweep_line Line = { InspectWidth, FirstLine, SecondLine, TestBlock, NoData };
        SetNoDataLine(FirstLine, InspectWidth, NoData, DType);
        for (int Row = 0; Row < Height; Row++)
        {
            GDALDatasetRasterIO(DS, GF_Read, 0, Row, Width, 1, SecondLine+1, Width, 1, DType, 1, 0, 0, InspectWidth * DTypeSize, 0);
            ProcessSweepLine(Line, Row, &EdgeData, ColHashTable);
            CopyData(FirstLine, InspectWidth, SecondLine, InspectWidth); // Copy SecondLine into FirstLine.
        }
        SetNoDataLine(SecondLine, InspectWidth, NoData, DType);
        ProcessSweepLine(Line, Height-1, &EdgeData, ColHashTable);
        
        FreeMemory(LineDataMem);
        
        StopTiming(&Trace);
        fprintf(stdout, "Sweep line: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Sort EdgeList according to connectedness
        
        usz NumEdges = EdgeData.WriteCur / sizeof(edge);
        u32* SortedEdges = (u32*)&EdgeData.Base[EdgeData.WriteCur];
        edge* EdgeList = (edge*)EdgeData.Base;
        for (usz EdgeIdx = 1; EdgeIdx < NumEdges; EdgeIdx++)
        {
            SortedEdges[EdgeIdx] = EdgeIdx;
            EdgeList[EdgeIdx].SortedIdx = EdgeIdx;
        }
        
        usz PolyAllocSize = (NumEdges+1) * sizeof(v2);
        Poly.Data = GetMemory(PolyAllocSize, 0, MEM_WRITE);
        if (!Poly.Data)
        {
            return Poly;
        }
        ring_info* Ring = (ring_info*)Poly.Data;
        
        WriteVertexPair(Ring, Affine, EdgeList, &SortedEdges[1]);
        
        edge _LowestEdgeStub = { EdgeType_None, I32_MAX, I32_MAX, 0, 0, 0 };
        edge _HighestEdgeStub = { EdgeType_None, (int)I32_MIN, (int)I32_MIN, 0, 0, 0 };
        
        u32 FirstIdx = 1;
        line_dir PrevDir = LineDir_Right; // Always begins to the right.
        
        StopTiming(&Trace);
        fprintf(stdout, "Pre-loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        for (u32 InsertIdx = 3; InsertIdx < NumEdges; InsertIdx += 2)
        {
            u32 NextIdx = 0;
            
            edge* PrevEdge = &EdgeList[SortedEdges[InsertIdx-1]];
            line_dir NextDir = GetLineDirection(PrevDir, PrevEdge);
            if (NextDir == LineDir_Down)
            {
                NextIdx = EdgeList[PrevEdge->Below].SortedIdx;
            }
            else
            {
                NextIdx = EdgeList[PrevEdge->Above].SortedIdx;
            }
            
            if (NextIdx == FirstIdx)
            {
                // Repeat first vertex of ring to close it.
                Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
                Poly.NumRings++;
                Poly.NumVertices += Ring->NumVertices;
                
                // New ring.
                Ring = GetNextRingInfo(Ring);
                WriteVertexPair(Ring, Affine, EdgeList, &SortedEdges[InsertIdx]);
                
                FirstIdx = InsertIdx;
                edge* FirstEdge = &EdgeList[SortedEdges[FirstIdx]];
                edge* SecondEdge = &EdgeList[SortedEdges[FirstIdx+1]];
                PrevDir = (SecondEdge->Col > FirstEdge->Col) ? LineDir_Right : LineDir_Left;
            }
            else
            {
                u32* Insert = &SortedEdges[InsertIdx];
                u32* Next = &SortedEdges[NextIdx];
                
                edge* InsertEdge = &EdgeList[Insert[0]];
                edge* NextEdge = &EdgeList[Next[0]];
                
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
                    
                    WriteVertexPair(Ring, Affine, EdgeList, Insert);
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
                    
                    WriteVertexPair(Ring, Affine, EdgeList, Insert);
                }
            }
        }
        
        Ring->Vertices[Ring->NumVertices++] = Ring->Vertices[0];
        Poly.NumRings++;
        Poly.NumVertices += Ring->NumVertices;
        
        StopTiming(&Trace);
        fprintf(stdout, "Loop: %f\n", Trace.Diff);
        StartTiming(&Trace);
        
        // Order polygons by outer/inner.
        Poly.Rings = (ring_info**)GetNextRingInfo(Ring);
        Ring = Poly.Rings[0] = (ring_info*)Poly.Data; // First ring is always outer.
        
        for (u32 Count = 1; Count < Poly.NumRings; Count++)
        {
            Ring = GetNextRingInfo(Ring);
#if 0
            ring_info* VerifyRing = Poly.Rings[0];
            for (u32 Count2 = 0; Count2 < Poly.NumRings; Count2++)
            {
                VerifyRing = GetNextRingInfo(VerifyRing);
            }
#else
            Poly.Rings[Count] = Ring;
#endif
        }
        
        FreeMemory(EdgeData.Base);
        
        StopTiming(&Trace);
        fprintf(stdout, "Post-loop: %f\n", Trace.Diff);
    }
    else
    {
        // Outline is raster BBox.
    }
    
    StartTiming(&Trace);
    GDALClose(DS);
    StopTiming(&Trace);
    fprintf(stdout, "Closing GDAL: %f\n", Trace.Diff);
    
    return Poly;
}