#include "PathFinder.h"

int pf_buildWaypoints(const int grid[PF_MAP_COLS][PF_MAP_ROWS],
                      PF_Waypoint *waypoints, int maxPoints,
                      int tileSize)
{
    int count = 0;

    /* 找到起点 */
    int startCol = -1, startRow = -1;
    for (int r = 0; r < PF_MAP_ROWS && startCol < 0; ++r)
    {
        for (int c = 0; c < PF_MAP_COLS; ++c)
        {
            if (grid[c][r] == 2)
            { /* 2 = Start */
                startCol = c;
                startRow = r;
                break;
            }
        }
    }
    if (startCol < 0 || count >= maxPoints)
        return 0;

    /* 添加起点 */
    waypoints[count].x = (float)(startCol * tileSize) + (float)tileSize / 2.0f;
    waypoints[count].y = (float)(startRow * tileSize) + (float)tileSize / 2.0f;
    ++count;

    /* 方向: 右 下 左 上 */
    const int dx[4] = {1, 0, -1, 0};
    const int dy[4] = {0, 1, 0, -1};

    int col = startCol, row = startRow;
    int prevCol = col, prevRow = row;
    int moving = 1;

    while (moving && count < maxPoints)
    {
        int found = 0;
        for (int d = 0; d < 4; ++d)
        {
            int nc = col + dx[d];
            int nr = row + dy[d];

            /* 不回头 */
            if (nc == prevCol && nr == prevRow)
                continue;

            if (nc >= 0 && nc < PF_MAP_COLS && nr >= 0 && nr < PF_MAP_ROWS)
            {
                int t = grid[nc][nr];
                if (t == 1 || t == 3)
                { /* 1=Path, 3=End */
                    prevCol = col;
                    prevRow = row;
                    col = nc;
                    row = nr;
                    waypoints[count].x = (float)(col * tileSize) + (float)tileSize / 2.0f;
                    waypoints[count].y = (float)(row * tileSize) + (float)tileSize / 2.0f;
                    ++count;
                    found = 1;
                    if (t == 3)
                        moving = 0; /* 到达终点 */
                    break;
                }
            }
        }
        if (!found)
            moving = 0;
    }

    return count;
}
