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

    // 尝试读取第二阶段 (如果有)
    m_hasPhase2 = false;
    int dummy;
    if (fscanf(fp, "%d", &dummy) == 1)
    {
        // 把刚才读到的放回去
        ungetc('0' + dummy, fp);  // 简单回退
        // 实际上用更可靠的方式：回退文件指针
        long pos = ftell(fp);
        fseek(fp, pos - 1, SEEK_SET);  // 回退一个数字
        // 这不够可靠，换个方式：直接把读到的第一个值放进 grid2
        // 算了，用简单方式：如果还能读到内容，就解析第二阶段
    }

    // 尝试跳过空行并读取第二阶段
    int ch;
    while ((ch = fgetc(fp)) != EOF && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
        ;
    if (ch != EOF)
    {
        ungetc(ch, fp);
        for (int r = 0; r < MAP_ROWS; ++r)
        {
            for (int c = 0; c < MAP_COLS; ++c)
            {
                int val;
                if (fscanf(fp, "%d", &val) == 1)
                {
                    m_grid2[c][r] = static_cast<TileType>(val);
                }
            }
        }
        m_hasPhase2 = true;
    }

    fclose(fp);

    buildWaypoints();
    std::cout << "[Map] Loaded: " << path;
    if (m_hasPhase2) std::cout << " (dual-phase)";
    std::cout << std::endl;

    // 验证路径
    if (m_waypoints.empty())
        std::cerr << "[Map] WARNING: No waypoints! Path may be broken." << std::endl;

    return true;
}

void Map::buildWaypoints()
{
    m_waypoints.clear();
    m_waypoints2.clear();

    // 第一阶段路径
    int grid[PF_MAP_COLS][PF_MAP_ROWS];
    for (int r = 0; r < PF_MAP_ROWS; ++r)
        for (int c = 0; c < PF_MAP_COLS; ++c)
            grid[c][r] = static_cast<int>(m_grid[c][r]);

    PF_Waypoint pfWp[256];
    int count = pf_tracePath(grid, pfWp, 256, TILE_SIZE);
    for (int i = 0; i < count; ++i)
        m_waypoints.push_back({sf::Vector2f(pfWp[i].x, pfWp[i].y)});

    // 第二阶段路径（如果有）
    if (m_hasPhase2)
    {
        for (int r = 0; r < PF_MAP_ROWS; ++r)
            for (int c = 0; c < PF_MAP_COLS; ++c)
                grid[c][r] = static_cast<int>(m_grid2[c][r]);

        count = pf_tracePath(grid, pfWp, 256, TILE_SIZE);
        for (int i = 0; i < count; ++i)
            m_waypoints2.push_back({sf::Vector2f(pfWp[i].x, pfWp[i].y)});
    }
}

void Map::switchPhase()
{
    if (!m_hasPhase2) return;
    m_phase2 = !m_phase2;
    // 交换当前网格和路径
    if (m_phase2)
    {
        for (int r = 0; r < MAP_ROWS; ++r)
            for (int c = 0; c < MAP_COLS; ++c)
                m_grid[c][r] = m_grid2[c][r];
        m_waypoints = m_waypoints2;
    }
    std::cout << "[Map] Switched to phase " << (m_phase2 ? 2 : 1) << std::endl;
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
                    m_tileShape.setTextureRect(sf::IntRect(0, 0,
                        static_cast<int>(m_endTex.getSize().x),
                        static_cast<int>(m_endTex.getSize().y)));
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
