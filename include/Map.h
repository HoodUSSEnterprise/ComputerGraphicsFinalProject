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
    void loadEndTextures();
    void loadBirthTexture();
    void loadTreasureTextures();
    void loadKongTexture(const std::string &biome);

    void draw(sf::RenderWindow &window) const;

    TileType getTile(int col, int row) const;
    void setTile(int col, int row, TileType type);

    sf::Vector2f gridToWorld(int col, int row) const;
    sf::Vector2i worldToGrid(float x, float y) const;

    bool canPlaceTower(int col, int row) const;

    // 宝藏系统
    int getTreasureHP(int col, int row) const;
    int getTreasureMaxHP(int col, int row) const;
    int getTreasureGold(int col, int row) const;
    int damageTreasure(int col, int row);  // 返回金币（0=没死或不是宝藏）
    bool isTreasure(int col, int row) const;
    bool isTreasureShowingHP(int col, int row) const;
    void initTreasureHP();
    void updateTreasureTimers(float dt);
    sf::Vector2f getTreasureWorldPos(int col, int row) const;

    const std::vector<Waypoint> &getWaypoints() const { return m_waypoints; }

private:
    void buildWaypoints();

    TileType m_grid[MAP_COLS][MAP_ROWS];
    std::vector<Waypoint> m_waypoints;

    sf::Texture m_groundTex;
    sf::Texture m_endTex;
    sf::Texture m_birthTex;
    sf::Texture m_kongTex;
    sf::Texture m_treasureTex[3];
    int m_treasureCount = 0;
    bool m_hasTexture = false;
    bool m_hasKongTex = false;

    // 宝藏血量：每个格子独立
    int m_treasureHP[MAP_COLS][MAP_ROWS] = {};
    int m_treasureMaxHP[MAP_COLS][MAP_ROWS] = {};
    int m_treasureIndex[MAP_COLS][MAP_ROWS] = {};  // 0/1/2 = 哪张图
    mutable float m_treasureLastHit[MAP_COLS][MAP_ROWS] = {};  // 上次受击后经过的时间

    mutable sf::Sprite m_groundSprite;
    mutable sf::Sprite m_endSprite;
    mutable sf::Sprite m_birthSprite;
    mutable sf::Sprite m_kongSprite;
    mutable sf::Sprite m_treasureSprites[3];
    mutable sf::RectangleShape m_tileShape;
    mutable sf::RectangleShape m_hpBarBg;
    mutable sf::RectangleShape m_hpBar;
};
