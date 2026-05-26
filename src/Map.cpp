#include "Map.h"
#include "PathFinder.h"
#include <SFML/Graphics.hpp>
#include <cstdio>
#include <iostream>

Map::Map() {
    // 初始化瓦片图形
    m_grassTile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_grassTile.setFillColor(sf::Color(34, 139, 34));
    m_grassTile.setOutlineColor(sf::Color(25, 100, 25));
    m_grassTile.setOutlineThickness(1);

    m_pathTile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_pathTile.setFillColor(sf::Color(194, 178, 128));
    m_pathTile.setOutlineColor(sf::Color(160, 140, 100));
    m_pathTile.setOutlineThickness(1);

    m_startTile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_startTile.setFillColor(sf::Color(0, 200, 0));

    m_endTile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_endTile.setFillColor(sf::Color(200, 0, 0));

    m_blockedTile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_blockedTile.setFillColor(sf::Color(100, 100, 100));
}

bool Map::loadFromFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        std::cerr << "[Map] Cannot open: " << path << std::endl;
        return false;
    }

    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            int val;
            if (fscanf(fp, "%d", &val) != 1) {
                std::cerr << "[Map] Read error at row " << r << " col " << c << std::endl;
                fclose(fp);
                return false;
            }
            m_grid[c][r] = static_cast<TileType>(val);
        }
    }

    fclose(fp);
    buildWaypoints();
    std::cout << "[Map] Loaded: " << path << std::endl;
    return true;
}

void Map::buildWaypoints() {
    m_waypoints.clear();

    // 将 grid 转为 int 数组传给 C 寻路函数
    int grid[PF_MAP_COLS][PF_MAP_ROWS];
    for (int r = 0; r < PF_MAP_ROWS; ++r)
        for (int c = 0; c < PF_MAP_COLS; ++c)
            grid[c][r] = static_cast<int>(m_grid[c][r]);

    PF_Waypoint pfWp[256];
    int count = pf_buildWaypoints(grid, pfWp, 256, TILE_SIZE);

    for (int i = 0; i < count; ++i) {
        m_waypoints.push_back({sf::Vector2f(pfWp[i].x, pfWp[i].y)});
    }
}

void Map::draw(sf::RenderWindow& window) const {
    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            float x = static_cast<float>(c * TILE_SIZE);
            float y = static_cast<float>(r * TILE_SIZE);

            sf::RectangleShape tile;
            switch (m_grid[c][r]) {
            case TileType::Grass:
                tile = m_grassTile;
                break;
            case TileType::Path:
                tile = m_pathTile;
                break;
            case TileType::Start:
                tile = m_startTile;
                break;
            case TileType::End:
                tile = m_endTile;
                break;
            case TileType::Blocked:
                tile = m_blockedTile;
                break;
            }
            tile.setPosition(x, y);
            window.draw(tile);
        }
    }
}

TileType Map::getTile(int col, int row) const {
    if (col >= 0 && col < MAP_COLS && row >= 0 && row < MAP_ROWS)
        return m_grid[col][row];
    return TileType::Blocked;
}

void Map::setTile(int col, int row, TileType type) {
    if (col >= 0 && col < MAP_COLS && row >= 0 && row < MAP_ROWS)
        m_grid[col][row] = type;
}

sf::Vector2f Map::gridToWorld(int col, int row) const {
    return sf::Vector2f(col * TILE_SIZE + TILE_SIZE / 2.0f,
                        row * TILE_SIZE + TILE_SIZE / 2.0f);
}

sf::Vector2i Map::worldToGrid(float x, float y) const {
    return sf::Vector2i(static_cast<int>(x / TILE_SIZE),
                        static_cast<int>(y / TILE_SIZE));
}

bool Map::canPlaceTower(int col, int row) const {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS)
        return false;
    return m_grid[col][row] == TileType::Grass;
}
