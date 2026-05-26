#include "Map.h"
#include "PathFinder.h"
#include <SFML/Graphics.hpp>
#include <cstdio>
#include <iostream>

Map::Map()
{
    m_tileShape.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
}

void Map::loadBiomeTextures(const std::string &biome)
{
    std::string path;
    if (biome == "grassland")
    {
        path = "textures/ground1.png";
    }
    else if (biome == "desert")
    {
        path = "textures/ground2.png";
    }
    else if (biome == "hell")
    {
        path = "textures/ground3.png";
    }
    else if (biome == "community")
    {
        path = "textures/ground1.png";
    }
    else
    {
        path = "textures/ground1.png";
    }

    m_hasTexture = m_groundTex.loadFromFile(path);
    if (m_hasTexture)
    {
        std::cout << "[Map] Texture loaded: " << path << std::endl;
    }
    else
    {
        std::cerr << "[Map] Texture FAILED: " << path << std::endl;
    }
}

void Map::loadEndTextures()
{
    if (m_endTex.loadFromFile("textures/endpoint.png"))
    {
        sf::Vector2u sz = m_endTex.getSize();
        std::cout << "[Map] endpoint.png loaded: " << sz.x << "x" << sz.y << std::endl;
    }
    else
    {
        std::cerr << "[Map] endpoint.png FAILED to load" << std::endl;
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

    // 验证路径：最后一个路径点应该接近终点
    if (m_waypoints.empty())
    {
        std::cerr << "[Map] WARNING: No waypoints! Path may be broken." << std::endl;
    }
    else
    {
        // 找到 End 格
        int endCol = -1, endRow = -1;
        for (int r = 0; r < MAP_ROWS && endCol < 0; ++r)
            for (int c = 0; c < MAP_COLS; ++c)
                if (m_grid[c][r] == TileType::End)
                {
                    endCol = c;
                    endRow = r;
                    break;
                }
        if (endCol >= 0)
        {
            sf::Vector2f endPos = gridToWorld(endCol, endRow);
            sf::Vector2f lastWp = m_waypoints.back().pos;
            float dx = lastWp.x - endPos.x;
            float dy = lastWp.y - endPos.y;
            if (dx * dx + dy * dy > TILE_SIZE * TILE_SIZE * 2)
            {
                std::cerr << "[Map] WARNING: Path may not reach End! Last waypoint at ("
                          << lastWp.x << "," << lastWp.y << "), End at ("
                          << endPos.x << "," << endPos.y << ")" << std::endl;
            }
        }
    }
    return true;
}

void Map::buildWaypoints()
{
    m_waypoints.clear();

    // 将 grid 转为 int 数组传给 C 寻路函数
    int grid[PF_MAP_COLS][PF_MAP_ROWS];
    for (int r = 0; r < PF_MAP_ROWS; ++r)
        for (int c = 0; c < PF_MAP_COLS; ++c)
            grid[c][r] = static_cast<int>(m_grid[c][r]);

    PF_Waypoint pfWp[256];
    int count = pf_tracePath(grid, pfWp, 256, TILE_SIZE);

    for (int i = 0; i < count; ++i)
    {
        m_waypoints.push_back({sf::Vector2f(pfWp[i].x, pfWp[i].y)});
    }
}

void Map::draw(sf::RenderWindow &window) const
{
    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            float x = static_cast<float>(c * TILE_SIZE);
            float y = static_cast<float>(r * TILE_SIZE);
            m_tileShape.setPosition(x, y);

            switch (m_grid[c][r])
            {
            case TileType::Grass:
                m_tileShape.setTexture(nullptr);
                m_tileShape.setFillColor(sf::Color::White);
                break;
            case TileType::Path:
                if (m_hasTexture)
                {
                    m_tileShape.setTexture(&m_groundTex);
                    m_tileShape.setTextureRect(sf::IntRect(0, 0, TILE_SIZE, TILE_SIZE));
                    m_tileShape.setFillColor(sf::Color(220, 220, 220));
                }
                else
                {
                    m_tileShape.setTexture(nullptr);
                    m_tileShape.setFillColor(sf::Color(194, 178, 128));
                }
                break;
            case TileType::Start:
                m_tileShape.setTexture(nullptr);
                m_tileShape.setFillColor(sf::Color(0, 200, 0));
                break;
            case TileType::End:
                if (m_endTex.getSize().x > 0)
                {
                    m_tileShape.setTexture(&m_endTex);
                    m_tileShape.setTextureRect(sf::IntRect(0, 0, TILE_SIZE, TILE_SIZE));
                    m_tileShape.setFillColor(sf::Color::White);
                }
                else
                {
                    m_tileShape.setTexture(nullptr);
                    m_tileShape.setFillColor(sf::Color(200, 0, 0));
                }
                break;
            case TileType::Blocked:
                m_tileShape.setTexture(nullptr);
                m_tileShape.setFillColor(sf::Color::Black);
                break;
            }
            window.draw(m_tileShape);
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
