#include "Map.h"
#include "PathFinder.h"
#include <SFML/Graphics.hpp>
#include <cstdio>
#include <iostream>

Map::Map()
{
    m_tileShape.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    m_hpBarBg.setSize(sf::Vector2f(TILE_SIZE * 0.6f, 4));
    m_hpBarBg.setFillColor(sf::Color(50, 50, 50));
    m_hpBar.setSize(sf::Vector2f(TILE_SIZE * 0.6f, 4));
    m_hpBar.setFillColor(sf::Color::Red);
}

void Map::loadBiomeTextures(const std::string &biome)
{
    std::string path;
    if (biome == "grassland")
        path = "textures/ground1.png";
    else if (biome == "desert")
        path = "textures/ground2.png";
    else if (biome == "hell")
        path = "textures/ground3.png";
    else if (biome == "community")
        path = "textures/ground1.png";
    else
        path = "textures/ground1.png";

    m_hasTexture = m_groundTex.loadFromFile(path);
    if (m_hasTexture)
    {
        m_groundTex.setRepeated(true);
        m_groundSprite = sf::Sprite(m_groundTex, sf::IntRect(0, 0, TILE_SIZE, TILE_SIZE));
        std::cout << "[Map] Ground texture loaded: " << path
                  << " (" << m_groundTex.getSize().x << "x" << m_groundTex.getSize().y << ")" << std::endl;
    }
    else
    {
        std::cerr << "[Map] Ground texture FAILED: " << path << std::endl;
    }
}

void Map::loadEndTextures()
{
    if (m_endTex.loadFromFile("textures/endpoint.png"))
    {
        m_endSprite = sf::Sprite(m_endTex);
        auto sz = m_endTex.getSize();
        float scale = TILE_SIZE * 0.9f / std::max(sz.x, sz.y);
        m_endSprite.setOrigin(sz.x / 2.0f, sz.y / 2.0f);
        m_endSprite.setScale(scale, scale);
        std::cout << "[Map] endpoint.png loaded: "
                  << sz.x << "x" << sz.y << std::endl;
    }
    else
    {
        std::cerr << "[Map] endpoint.png FAILED to load" << std::endl;
    }
}

void Map::loadBirthTexture()
{
    if (m_birthTex.loadFromFile("textures/birth.png"))
    {
        m_birthSprite = sf::Sprite(m_birthTex);
        auto sz = m_birthTex.getSize();
        float scale = TILE_SIZE * 0.9f / std::max(sz.x, sz.y);
        m_birthSprite.setOrigin(sz.x / 2.0f, sz.y / 2.0f);
        m_birthSprite.setScale(scale, scale);
        std::cout << "[Map] birth.png loaded: "
                  << sz.x << "x" << sz.y << std::endl;
    }
    else
    {
        std::cerr << "[Map] birth.png FAILED to load" << std::endl;
    }
}

void Map::loadTreasureTextures()
{
    const char *paths[] = {"textures/Treasure1.png", "textures/Treasure2.png", "textures/Treasure3.png"};
    m_treasureCount = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (m_treasureTex[i].loadFromFile(paths[i]))
        {
            m_treasureSprites[i] = sf::Sprite(m_treasureTex[i]);
            m_treasureCount++;
            std::cout << "[Map] " << paths[i] << " loaded: "
                      << m_treasureTex[i].getSize().x << "x" << m_treasureTex[i].getSize().y << std::endl;
        }
        else
        {
            std::cerr << "[Map] " << paths[i] << " FAILED to load" << std::endl;
        }
    }
}

bool Map::loadFromFile(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        std::cerr << "[Map] Cannot open: " << path << std::endl;
        return false;
    }

    // 读取第一阶段 (12行)
    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            int val;
            if (fscanf(fp, "%d", &val) != 1)
            {
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

    if (m_waypoints.empty())
        std::cerr << "[Map] WARNING: No waypoints! Path may be broken." << std::endl;

    return true;
}

void Map::buildWaypoints()
{
    m_waypoints.clear();

    int grid[PF_MAP_COLS][PF_MAP_ROWS];
    for (int r = 0; r < PF_MAP_ROWS; ++r)
        for (int c = 0; c < PF_MAP_COLS; ++c)
            grid[c][r] = static_cast<int>(m_grid[c][r]);

    PF_Waypoint pfWp[256];
    int count = pf_tracePath(grid, pfWp, 256, TILE_SIZE);
    for (int i = 0; i < count; ++i)
        m_waypoints.push_back({sf::Vector2f(pfWp[i].x, pfWp[i].y)});
}

void Map::draw(sf::RenderWindow &window) const
{
    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            float x = static_cast<float>(c * TILE_SIZE);
            float y = static_cast<float>(r * TILE_SIZE);

            switch (m_grid[c][r])
            {
            case TileType::Grass:
                // 纯白底色（Grass 不可见纹理）
                m_tileShape.setPosition(x, y);
                m_tileShape.setTexture(nullptr);
                m_tileShape.setFillColor(sf::Color::White);
                window.draw(m_tileShape);
                break;

            case TileType::Path:
                if (m_hasTexture)
                {
                    m_groundSprite.setPosition(x, y);
                    window.draw(m_groundSprite);
                }
                else
                {
                    m_tileShape.setPosition(x, y);
                    m_tileShape.setTexture(nullptr);
                    m_tileShape.setFillColor(sf::Color(194, 178, 128));
                    window.draw(m_tileShape);
                }
                break;

            case TileType::Start:
                // 先画地面纹理
                if (m_hasTexture)
                {
                    m_groundSprite.setPosition(x, y);
                    window.draw(m_groundSprite);
                }
                // 叠上出生点精灵
                if (m_birthTex.getSize().x > 0)
                {
                    m_birthSprite.setPosition(x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f);
                    window.draw(m_birthSprite);
                }
                break;

            case TileType::End:
                // 先画地面纹理
                if (m_hasTexture)
                {
                    m_groundSprite.setPosition(x, y);
                    window.draw(m_groundSprite);
                }
                // 叠上终点精灵
                if (m_endTex.getSize().x > 0)
                {
                    m_endSprite.setPosition(x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f);
                    window.draw(m_endSprite);
                }
                break;

            case TileType::Blocked:
                // 白色底色
                m_tileShape.setPosition(x, y);
                m_tileShape.setTexture(nullptr);
                m_tileShape.setFillColor(sf::Color::White);
                window.draw(m_tileShape);
                // 叠上宝藏（有血量才画）
                if (m_treasureCount > 0 && m_treasureHP[c][r] > 0)
                {
                    int pick = m_treasureIndex[c][r];
                    auto &spr = m_treasureSprites[pick];
                    auto tsz = m_treasureTex[pick].getSize();
                    float scale = TILE_SIZE * 0.7f / std::max(tsz.x, tsz.y);
                    spr.setOrigin(tsz.x / 2.0f, tsz.y / 2.0f);
                    spr.setScale(scale, scale);
                    spr.setPosition(x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f);
                    window.draw(spr);

                    // 血条（仅受击后3秒内显示）
                    if (isTreasureShowingHP(c, r))
                    {
                        float hpRatio = static_cast<float>(m_treasureHP[c][r]) / m_treasureMaxHP[c][r];
                        m_hpBarBg.setPosition(x + TILE_SIZE * 0.2f, y + TILE_SIZE - 8);
                        m_hpBar.setSize(sf::Vector2f(TILE_SIZE * 0.6f * hpRatio, 4));
                        m_hpBar.setPosition(x + TILE_SIZE * 0.2f, y + TILE_SIZE - 8);
                        if (hpRatio > 0.5f) m_hpBar.setFillColor(sf::Color::Green);
                        else if (hpRatio > 0.25f) m_hpBar.setFillColor(sf::Color::Yellow);
                        else m_hpBar.setFillColor(sf::Color::Red);
                        window.draw(m_hpBarBg);
                        window.draw(m_hpBar);
                    }
                }
                break;
            }
        }
    }
}

TileType Map::getTile(int col, int row) const
{
    if (col >= 0 && col < MAP_COLS && row >= 0 && row < MAP_ROWS)
        return m_grid[col][row];
    return TileType::Blocked;
}

void Map::setTile(int col, int row, TileType type)
{
    if (col >= 0 && col < MAP_COLS && row >= 0 && row < MAP_ROWS)
        m_grid[col][row] = type;
}

sf::Vector2f Map::gridToWorld(int col, int row) const
{
    return sf::Vector2f(col * TILE_SIZE + TILE_SIZE / 2.0f,
                        row * TILE_SIZE + TILE_SIZE / 2.0f);
}

sf::Vector2i Map::worldToGrid(float x, float y) const
{
    return sf::Vector2i(static_cast<int>(x / TILE_SIZE),
                        static_cast<int>(y / TILE_SIZE));
}

bool Map::canPlaceTower(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS)
        return false;
    return m_grid[col][row] == TileType::Grass;
}

// ============================================================
//  宝藏系统
// ============================================================

void Map::initTreasureHP()
{
    int golds[] = {50, 100, 150};
    int hps[]   = {3, 5, 8};

    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            m_treasureHP[c][r] = 0;
            m_treasureMaxHP[c][r] = 0;
            m_treasureIndex[c][r] = -1;

            if (m_grid[c][r] == TileType::Blocked && m_treasureCount > 0)
            {
                int pick = (c * 7 + r * 13) % m_treasureCount;
                m_treasureIndex[c][r] = pick;
                m_treasureMaxHP[c][r] = hps[pick];
                m_treasureHP[c][r] = hps[pick];
            }
        }
    }
}

bool Map::isTreasure(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
    return m_grid[col][row] == TileType::Blocked && m_treasureHP[col][row] > 0;
}

int Map::getTreasureHP(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return 0;
    return m_treasureHP[col][row];
}

int Map::getTreasureMaxHP(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return 0;
    return m_treasureMaxHP[col][row];
}

int Map::getTreasureGold(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return 0;
    int idx = m_treasureIndex[col][row];
    if (idx < 0) return 0;
    int golds[] = {50, 100, 150};
    return golds[idx];
}

int Map::damageTreasure(int col, int row)
{
    if (!isTreasure(col, row)) return 0;

    m_treasureHP[col][row]--;
    m_treasureLastHit[col][row] = 0;  // 重置计时器

    if (m_treasureHP[col][row] <= 0)
    {
        int gold = getTreasureGold(col, row);
        m_grid[col][row] = TileType::Grass;
        m_treasureHP[col][row] = 0;
        m_treasureLastHit[col][row] = 99;  // 销毁后不再显示
        return gold;
    }
    return 0;
}

bool Map::isTreasureShowingHP(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
    return m_treasureHP[col][row] > 0 && m_treasureLastHit[col][row] < 3.0f;
}

void Map::updateTreasureTimers(float dt)
{
    for (int r = 0; r < MAP_ROWS; ++r)
        for (int c = 0; c < MAP_COLS; ++c)
            if (m_treasureHP[c][r] > 0 && m_treasureLastHit[c][r] < 99)
                m_treasureLastHit[c][r] += dt;
}

sf::Vector2f Map::getTreasureWorldPos(int col, int row) const
{
    return sf::Vector2f(col * TILE_SIZE + TILE_SIZE / 2.0f,
                        row * TILE_SIZE + TILE_SIZE / 2.0f);
}
