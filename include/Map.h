#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct Waypoint
{
    sf::Vector2f pos;
};

class Map
{
public:
    Map();
    bool loadFromFile(const char *path);
    void loadBiomeTextures(const std::string &biome);

    void draw(sf::RenderWindow &window) const;

    TileType getTile(int col, int row) const;
    void setTile(int col, int row, TileType type);

    sf::Vector2f gridToWorld(int col, int row) const;
    sf::Vector2i worldToGrid(float x, float y) const;

    bool canPlaceTower(int col, int row) const;

    const std::vector<Waypoint> &getWaypoints() const { return m_waypoints; }

private:
    void buildWaypoints();

    TileType m_grid[MAP_COLS][MAP_ROWS];
    std::vector<Waypoint> m_waypoints;

    sf::Texture m_groundTex;
    sf::Texture m_endTex;
    bool m_hasTexture = false;

    mutable sf::RectangleShape m_tileShape;
};
