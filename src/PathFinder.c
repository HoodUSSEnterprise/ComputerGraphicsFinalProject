#include "PathFinder.h"

/* 沿路径线性跟踪：从起点逐格走到终点 */
int pf_buildWaypoints(const int grid[PF_MAP_COLS][PF_MAP_ROWS],
                      PF_Waypoint *waypoints, int maxPoints,
                      int tileSize)
{
    int count = 0;
    const float half = (float)tileSize / 2.0f;

    /* 找到起点 */
    int col = -1, row = -1;
    for (int r = 0; r < PF_MAP_ROWS && col < 0; ++r)
        for (int c = 0; c < PF_MAP_COLS; ++c)
            if (grid[c][r] == 2) { col = c; row = r; break; }
    if (col < 0 || count >= maxPoints) return 0;

    /* 记录起点 */
    waypoints[count].x = (float)(col * tileSize) + half;
    waypoints[count].y = (float)(row * tileSize) + half;
    ++count;

    int prevCol = col, prevRow = row;

    /* 四个方向：右 下 左 上 */
    const int dx[4] = {1, 0, -1, 0};
    const int dy[4] = {0, 1, 0, -1};

    for (;;) {
        int found = 0;
        for (int d = 0; d < 4; ++d) {
            int nc = col + dx[d];
            int nr = row + dy[d];
            if (nc == prevCol && nr == prevRow) continue;           /* 不回头 */
            if (nc < 0 || nc >= PF_MAP_COLS) continue;
            if (nr < 0 || nr >= PF_MAP_ROWS) continue;

            int t = grid[nc][nr];
            if (t == 1 || t == 3) {                                  /* 路径或终点 */
                prevCol = col; prevRow = row;
                col = nc;   row  = nr;
                waypoints[count].x = (float)(col * tileSize) + half;
                waypoints[count].y = (float)(row * tileSize) + half;
                ++count;
                found = 1;
                if (t == 3) return count;                            /* 到达终点 */
                break;
            }
        }
        if (!found || count >= maxPoints) break;
    }
    return count;
}