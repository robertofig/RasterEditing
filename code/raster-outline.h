#ifndef RASTER_OUTLINE_H
#define RASTER_OUTLINE_H

enum edge_type
{
    EdgeType_TopLeft = 0,
    EdgeType_TopRight = 1,
    EdgeType_BottomLeft = 2,
    EdgeType_BottomRight = 3,
    EdgeType_None = 4
};

#define EDGE_DATA_START_SIZE Kilobyte(64)

typedef edge_type (*test_block)(u8*, u8*, double);

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

struct sweep_info
{
    u32 LineSweepSize;
    u8* FirstLine;
    u8* SecondLine;
    test_block TestBlock;
    double NoData;
    
    u32* ColHashTable;
};

struct edge_data
{
    u32 Count;
    edge List[0];
};

struct ring_info
{
    ring_info* Next;
    
    u32 Type; // 0: Outer, 1: Inner
    u32 NumVertices;
    v2 Vertices[0];
};

#define RING_DATA_START_SIZE (256 * sizeof(ring_info))

struct poly_info
{
    u32 NumVertices;
    u32 NumRings;
    ring_info* Rings;
};

#endif //RASTER_OUTLINE_H
