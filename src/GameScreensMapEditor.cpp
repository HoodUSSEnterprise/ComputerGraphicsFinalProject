// 地图编辑器实现
#include "Game.h"
#include "LangManager.h"
#include <fstream>
#include <iostream>

void Game::buildMapEditorUI()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 初始化空白地图（全 Grass = kong 铺满）
    for (int r = 0; r < MAP_ROWS; ++r)
        for (int c = 0; c < MAP_COLS; ++c)
            m_editMap.setTile(c, r, TileType::Grass);

    // 加载 kong1 作为编辑器背景
    m_editKongTex.loadFromFile("textures/kong1.png");
    m_editKongTex.setRepeated(true);
    m_editKongSprite = sf::Sprite(m_editKongTex, sf::IntRect(0, 0, TILE_SIZE, TILE_SIZE));

    m_editTool = TileType::Path;

    // 工具按钮（底部）
    m_editToolBtns.clear();
    struct { TileType type; const wchar_t* name; sf::Color color; } tools[] = {
        {TileType::Path,    L"路径",  sf::Color(194, 178, 128)},
        {TileType::Start,   L"起点",  sf::Color(0, 200, 0)},
        {TileType::End,     L"终点",  sf::Color(200, 0, 0)},
        {TileType::Blocked, L"宝藏",  sf::Color(255, 215, 0)},
        {TileType::Grass,   L"空地",  sf::Color(200, 200, 200)},
    };

    float tx = 30;
    for (int i = 0; i < 5; ++i)
    {
        EditToolBtn tb;
        tb.type = tools[i].type;
        tb.bg.setSize(sf::Vector2f(90, 36));
        tb.bg.setPosition(tx, MAP_ROWS * TILE_SIZE + 32);
        tb.bg.setFillColor(tools[i].color);
        tb.bg.setOutlineColor(sf::Color::White);
        tb.bg.setOutlineThickness(i == 0 ? 3 : 1);
        tb.label.setFont(m_menuFont);
        tb.label.setString(tools[i].name);
        tb.label.setCharacterSize(14);
        tb.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = tb.label.getLocalBounds();
        tb.label.setOrigin(lb.width / 2, lb.height / 2);
        tb.label.setPosition(tx + 45, MAP_ROWS * TILE_SIZE + 50);
        m_editToolBtns.push_back(tb);
        tx += 105;
    }

    // 保存按钮
    m_editSaveBtn.setSize(sf::Vector2f(100, 36));
    m_editSaveBtn.setPosition(WINDOW_WIDTH - 220, MAP_ROWS * TILE_SIZE + 32);
    m_editSaveBtn.setTexture(&m_buttonTex);
    m_editSaveBtn.setFillColor(sf::Color(180, 255, 180));
    m_editSaveBtn.setOutlineThickness(0);
    m_editSaveLabel.setFont(m_menuFont);
    m_editSaveLabel.setString(L"Save");
    m_editSaveLabel.setCharacterSize(16);
    m_editSaveLabel.setFillColor(sf::Color::White);

    // 返回按钮
    m_editBackBtn.setSize(sf::Vector2f(100, 36));
    m_editBackBtn.setPosition(WINDOW_WIDTH - 110, MAP_ROWS * TILE_SIZE + 32);
    m_editBackBtn.setTexture(&m_buttonTex);
    m_editBackBtn.setFillColor(sf::Color(200, 200, 220));
    m_editBackBtn.setOutlineThickness(0);
    m_editBackLabel.setFont(m_menuFont);
    m_editBackLabel.setString(LangManager::get(TextKey::Back));
    m_editBackLabel.setCharacterSize(16);
    m_editBackLabel.setFillColor(sf::Color::White);
}

void Game::renderMapEditor()
{
    m_window.clear(sf::Color(20, 20, 30));

    // 绘制网格
    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            float x = static_cast<float>(c * TILE_SIZE);
            float y = static_cast<float>(r * TILE_SIZE);

            // kong 底色
            m_editKongSprite.setPosition(x, y);
            m_window.draw(m_editKongSprite);

            TileType t = m_editMap.getTile(c, r);
            if (t == TileType::Path)
            {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(100, 80, 60, 180));
                m_window.draw(rect);
            }
            else if (t == TileType::Start)
            {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(0, 200, 0, 160));
                m_window.draw(rect);
                sf::Text txt;
                txt.setFont(m_menuFont);
                txt.setString(L"S");
                txt.setCharacterSize(24);
                txt.setFillColor(sf::Color::White);
                sf::FloatRect tb = txt.getLocalBounds();
                txt.setOrigin(tb.width / 2, tb.height / 2);
                txt.setPosition(x + TILE_SIZE / 2, y + TILE_SIZE / 2);
                m_window.draw(txt);
            }
            else if (t == TileType::End)
            {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(200, 0, 0, 160));
                m_window.draw(rect);
                sf::Text txt;
                txt.setFont(m_menuFont);
                txt.setString(L"E");
                txt.setCharacterSize(24);
                txt.setFillColor(sf::Color::White);
                sf::FloatRect tb = txt.getLocalBounds();
                txt.setOrigin(tb.width / 2, tb.height / 2);
                txt.setPosition(x + TILE_SIZE / 2, y + TILE_SIZE / 2);
                m_window.draw(txt);
            }
            else if (t == TileType::Blocked)
            {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(255, 215, 0, 140));
                m_window.draw(rect);
            }

            // 网格线
            sf::RectangleShape gridLine;
            gridLine.setFillColor(sf::Color(255, 255, 255, 20));
            if (c < MAP_COLS - 1)
            {
                gridLine.setSize(sf::Vector2f(1, TILE_SIZE));
                gridLine.setPosition(x + TILE_SIZE - 1, y);
                m_window.draw(gridLine);
            }
            if (r < MAP_ROWS - 1)
            {
                gridLine.setSize(sf::Vector2f(TILE_SIZE, 1));
                gridLine.setPosition(x, y + TILE_SIZE - 1);
                m_window.draw(gridLine);
            }
        }
    }

    // 底部 UI 背景
    sf::RectangleShape uiBg(sf::Vector2f(WINDOW_WIDTH, 100));
    uiBg.setPosition(0, MAP_ROWS * TILE_SIZE);
    uiBg.setFillColor(sf::Color(20, 20, 40));
    m_window.draw(uiBg);

    // 工具按钮
    for (auto &tb : m_editToolBtns)
    {
        m_window.draw(tb.bg);
        m_window.draw(tb.label);
    }

    // 保存按钮
    m_window.draw(m_editSaveBtn);
    sf::FloatRect slb = m_editSaveLabel.getLocalBounds();
    m_editSaveLabel.setOrigin(slb.width / 2, slb.height / 2);
    m_editSaveLabel.setPosition(WINDOW_WIDTH - 170, MAP_ROWS * TILE_SIZE + 50);
    m_window.draw(m_editSaveLabel);

    // 返回按钮
    m_window.draw(m_editBackBtn);
    sf::FloatRect blb = m_editBackLabel.getLocalBounds();
    m_editBackLabel.setOrigin(blb.width / 2, blb.height / 2);
    m_editBackLabel.setPosition(WINDOW_WIDTH - 60, MAP_ROWS * TILE_SIZE + 50);
    m_window.draw(m_editBackLabel);
}

void Game::processMapEditorEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        // 不需要 hover 处理
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    // 点击工具按钮
    for (size_t i = 0; i < m_editToolBtns.size(); ++i)
    {
        if (m_editToolBtns[i].bg.getGlobalBounds().contains(mx, my))
        {
            m_editTool = m_editToolBtns[i].type;
            for (auto &tb : m_editToolBtns)
                tb.bg.setOutlineThickness(1);
            m_editToolBtns[i].bg.setOutlineThickness(3);
            return;
        }
    }

    // 点击保存按钮
    if (m_editSaveBtn.getGlobalBounds().contains(mx, my))
    {
        // 保存到 custom 目录
        std::filesystem::create_directories("assets/maps/custom");
        int num = 1;
        std::string path;
        do {
            path = "assets/maps/custom/custom" + std::to_string(num) + ".txt";
            num++;
        } while (std::filesystem::exists(path));

        std::ofstream f(path);
        if (f)
        {
            for (int r = 0; r < MAP_ROWS; ++r)
            {
                for (int c = 0; c < MAP_COLS; ++c)
                    f << static_cast<int>(m_editMap.getTile(c, r)) << (c < MAP_COLS - 1 ? " " : "");
                f << (r < MAP_ROWS - 1 ? "\n" : "");
            }
            std::cout << "[MapEditor] Saved: " << path << std::endl;
        }
        m_state = GameState::Menu;
        return;
    }

    // 点击返回按钮
    if (m_editBackBtn.getGlobalBounds().contains(mx, my))
    {
        m_state = GameState::Menu;
        return;
    }

    // 点击网格放置
    int col = static_cast<int>(mx / TILE_SIZE);
    int row = static_cast<int>(my / TILE_SIZE);
    if (col >= 0 && col < MAP_COLS && row >= 0 && row < MAP_ROWS)
    {
        m_editMap.setTile(col, row, m_editTool);
    }
}
