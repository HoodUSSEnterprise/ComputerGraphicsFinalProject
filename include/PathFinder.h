#ifndef PATHFINDER_H
#define PATHFINDER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PF_MAP_COLS 16
#define PF_MAP_ROWS 12

    // 0=草地, 1=路径, 2=起点, 3=终点, 4=障碍
    typedef struct
    {
        float x, y;
    } PF_Waypoint;

    // 从 Start(2) 沿 Path(1) 走到 End(3)，返回路径点数量
    int pf_tracePath(const int grid[PF_MAP_COLS][PF_MAP_ROWS],
                          PF_Waypoint *waypoints, int maxPoints,
                          int tileSize);

#ifdef __cplusplus
}
#endif

#endif
