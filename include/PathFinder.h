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

    // 从 grid[col][row] 构建路径点数组，返回路径点数量，waypoints 需预分配足够空间
    int pf_buildWaypoints(const int grid[PF_MAP_COLS][PF_MAP_ROWS],
                          PF_Waypoint *waypoints, int maxPoints,
                          int tileSize);

#ifdef __cplusplus
}
#endif

#endif
