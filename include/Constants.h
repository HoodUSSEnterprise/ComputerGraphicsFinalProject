#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// 常量定义
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr int TILE_SIZE = 64;
constexpr int MAP_COLS = 16;
constexpr int MAP_ROWS = 12;

// 地图格类型
enum class TileType : int {
    Grass = 0,   // 可放置塔
    Path = 1,    // 敌人路径
    Start = 2,   // 出生点
    End = 3,     // 终点
    Blocked = 4  // 不可放置
};
