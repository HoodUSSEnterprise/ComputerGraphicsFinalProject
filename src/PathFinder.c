#include "PathFinder.h"

/* 从 Start(2) 出发，沿 Path(1) 逐格走到 End(3)，记录路径点 */
int pf_tracePath(const int grid[PF_MAP_COLS][PF_MAP_ROWS],
                 PF_Waypoint *waypoints, int maxPoints,
                 int tileSize)
{
    int count = 0;
    const float half = (float)tileSize / 2.0f;

    /* 找起点(2) */
    int col = -1, row = -1;
    for (int r = 0; r < PF_MAP_ROWS && col < 0; ++r)
    {
        for (int c = 0; c < PF_MAP_COLS; ++c)
        {
            if (grid[c][r] == 2)
            {
                col = c;
                row = r;
                break;
            }
        }
    }
    if (col < 0)
    {
        return 0;
    }

    waypoints[count].x = (float)(col * tileSize) + half;
    waypoints[count].y = (float)(row * tileSize) + half;
    ++count;

    int prevCol = col, prevRow = row;
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};

    while (count < maxPoints)
    {
        int moved = 0;
        for (int d = 0; d < 4; ++d)
        {
            int nc = col + dx[d];
            int nr = row + dy[d];
            if (nc == prevCol && nr == prevRow)
                continue;
            if (nc < 0 || nc >= PF_MAP_COLS || nr < 0 || nr >= PF_MAP_ROWS)
                continue;

            int t = grid[nc][nr];
            if (t == 1 || t == 3)
            {
                prevCol = col;
                prevRow = row;
                col = nc;
                row = nr;
                waypoints[count].x = (float)(col * tileSize) + half;
                waypoints[count].y = (float)(row * tileSize) + half;
                ++count;
                moved = 1;
                if (t == 3)
                    return count;
                break;
            }
        }
        if (!moved)
            break;
    }
    return count;
}