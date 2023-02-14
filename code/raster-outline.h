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

#define EDGE_LIST_START_SIZE Kilobyte(64)
#define RING_LIST_START_SIZE Kilobyte(4)

typedef edge_type (*test_block)(u8*, u8*, double);
struct dtype_info
{
    int Size;
    test_block Func;
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
    LineDir_Up
};

struct sweep_line
{
    int Width;
    u8* First;
    u8* Second;
};

struct ring_info
{
    u32 Type; // 0: Outer, 1: Inner
    u32 NumVertices;
    v2 Vertices[0];
};

struct poly_info
{
    void* Data;
    
    u32 NumVertices;
    u32 NumRings;
    ring_info** Rings;
};

#endif //RASTER_OUTLINE_H
