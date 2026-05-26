#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct Waypoint {
    sf::Vector2f pos;
};

class Map {
public:
    Map();

    void draw(sf::RenderWindow& window) const;

    TileType getTile(int col, int row) const;
    void setTile(int col, int row, TileType type);

    sf::Vector2f gridToWorld(int col, int row) const;
    sf::Vector2i worldToGrid(float x, float y) const;

    bool canPlaceTower(int col, int row) const;

    const std::vector<Waypoint>& getWaypoints() const { return m_waypoints; }

private:
    void loadDefaultMap();
    void buildWaypoints();

    TileType m_grid[MAP_COLS][MAP_ROWS];
    std::vector<Waypoint> m_waypoints;

    sf::RectangleShape m_grassTile;
    sf::RectangleShape m_pathTile;
    sf::RectangleShape m_startTile;
    sf::RectangleShape m_endTile;
    sf::RectangleShape m_blockedTile;
};
