// 地图编辑器
#include "Game.h"
#include "LangManager.h"
#include <fstream>
#include <iostream>

static const int EDITOR_COLS = MAP_COLS;
static const int EDITOR_ROWS = MAP_ROWS;

void Game::enterMapEditor()
{
    // 初始化全 kong (Grass)
    for (int r = 0; r < EDITOR_ROWS; ++r)
        for (int c = 0; c < EDITOR_COLS; ++c)
            m_editGrid[c][r] = TileType::Grass;
    m_editMode = 1; // 默认 Path 模式
    m_editorMsg.clear();
    m_state = GameState::MapEditor;
}

void Game::renderMapEditor()
{
    drawBackground();
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    m_window.draw(bg);

    // 绘制网格
    for (int r = 0; r < EDITOR_ROWS; ++r)
    {
        for (int c = 0; c < EDITOR_COLS; ++c)
        {
            float x = static_cast<float>(c * TILE_SIZE);
            float y = static_cast<float>(r * TILE_SIZE);

            // 背景
            m_tileShape.setPosition(x, y);
            m_tileShape.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));

            switch (m_editGrid[c][r])
            {
            case TileType::Grass:
                m_tileShape.setFillColor(sf::Color(60, 80, 60));
                break;
            case TileType::Path:
                m_tileShape.setFillColor(sf::Color(160, 140, 100));
                break;
            case TileType::Start:
                m_tileShape.setFillColor(sf::Color(0, 200, 0));
                break;
            case TileType::End:
                m_tileShape.setFillColor(sf::Color(200, 0, 0));
                break;
            case TileType::Blocked:
                m_tileShape.setFillColor(sf::Color(80, 60, 40));
                break;
            }
            m_window.draw(m_tileShape);

            // 网格线
            sf::RectangleShape border(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            border.setPosition(x, y);
            border.setFillColor(sf::Color::Transparent);
            border.setOutlineColor(sf::Color(100, 100, 100, 60));
            border.setOutlineThickness(1);
            m_window.draw(border);

            // 显示编号
            if (m_editGrid[c][r] == TileType::Start)
            {
                sf::Text t;
                t.setFont(m_menuFont);
                t.setString("S");
                t.setCharacterSize(20);
                t.setFillColor(sf::Color::White);
                t.setPosition(x + 22, y + 18);
                m_window.draw(t);
            }
            else if (m_editGrid[c][r] == TileType::End)
            {
                sf::Text t;
                t.setFont(m_menuFont);
                t.setString("E");
                t.setCharacterSize(20);
                t.setFillColor(sf::Color::White);
                t.setPosition(x + 22, y + 18);
                m_window.draw(t);
            }
        }
    }

    // 右侧面板
    float panelX = EDITOR_COLS * TILE_SIZE + 10;
    float panelY = 10;

    // 模式选择
    const char *modeNames[] = {"Path", "Start", "End", "Blocked"};
    sf::Color modeColors[] = {
        sf::Color(160, 140, 100),
        sf::Color(0, 200, 0),
        sf::Color(200, 0, 0),
        sf::Color(80, 60, 40)
    };

    for (int i = 0; i < 4; ++i)
    {
        sf::RectangleShape btn(sf::Vector2f(180, 36));
        btn.setPosition(panelX, panelY + i * 42);
        btn.setFillColor(m_editMode == i + 1 ? sf::Color(255, 215, 0, 80) : sf::Color(50, 50, 70));
        btn.setOutlineColor(m_editMode == i + 1 ? sf::Color::Yellow : sf::Color(100, 100, 140));
        btn.setOutlineThickness(1);
        m_window.draw(btn);

        sf::Text label;
        label.setFont(m_menuFont);
        label.setString(modeNames[i]);
        label.setCharacterSize(16);
        label.setFillColor(sf::Color::White);
        label.setPosition(panelX + 8, panelY + i * 42 + 6);
        m_window.draw(label);
    }

    // 保存按钮
    {
        sf::RectangleShape btn(sf::Vector2f(180, 40));
        btn.setPosition(panelX, panelY + 180);
        btn.setTexture(&m_buttonTex);
        btn.setFillColor(sf::Color(180, 255, 180));
        m_window.draw(btn);
        sf::Text label;
        label.setFont(m_menuFont);
        label.setString("Save Map");
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);
        sf::FloatRect lb = label.getLocalBounds();
        label.setOrigin(lb.width / 2, lb.height / 2);
        label.setPosition(panelX + 90, panelY + 200);
        m_window.draw(label);
    }

    // 返回按钮
    {
        sf::RectangleShape btn(sf::Vector2f(180, 40));
        btn.setPosition(panelX, panelY + 230);
        btn.setTexture(&m_buttonTex);
        btn.setFillColor(sf::Color(255, 180, 180));
        m_window.draw(btn);
        sf::Text label;
        label.setFont(m_menuFont);
        label.setString("Back");
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);
        sf::FloatRect lb = label.getLocalBounds();
        label.setOrigin(lb.width / 2, lb.height / 2);
        label.setPosition(panelX + 90, panelY + 250);
        m_window.draw(label);
    }

    // 消息
    if (!m_editorMsg.empty())
    {
        sf::Text msg;
        msg.setFont(m_menuFont);
        msg.setString(m_editorMsg);
        msg.setCharacterSize(16);
        msg.setFillColor(sf::Color(255, 200, 100));
        msg.setPosition(panelX, panelY + 290);
        m_window.draw(msg);
    }

    // 提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString("ESC: Back to Menu");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    hint.setPosition(panelX, WINDOW_HEIGHT + 60);
    m_window.draw(hint);
}

void Game::processMapEditorEvents(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_state = GameState::Menu;
            return;
        }
        // 数字键切换模式
        if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num4)
        {
            m_editMode = event.key.code - sf::Keyboard::Num1 + 1;
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    float panelX = EDITOR_COLS * TILE_SIZE + 10;
    float panelY = 10;

    // 右侧面板按钮
    // 模式按钮
    for (int i = 0; i < 4; ++i)
    {
        if (mx >= panelX && mx <= panelX + 180 && my >= panelY + i * 42 && my <= panelY + i * 42 + 36)
        {
            m_editMode = i + 1;
            return;
        }
    }

    // 保存按钮
    if (mx >= panelX && mx <= panelX + 180 && my >= panelY + 180 && my <= panelY + 220)
    {
        // 检查是否有 Start 和 End
        int startCount = 0, endCount = 0;
        for (int r = 0; r < EDITOR_ROWS; ++r)
            for (int c = 0; c < EDITOR_COLS; ++c)
            {
                if (m_editGrid[c][r] == TileType::Start) startCount++;
                if (m_editGrid[c][r] == TileType::End) endCount++;
            }

        if (startCount != 1 || endCount != 1)
        {
            m_editorMsg = L"Need exactly 1 Start and 1 End!";
            return;
        }

        // 保存到源码 assets/maps/
        std::string path = "../assets/maps/custom_editor.txt";
        std::ofstream f(path);
        if (!f) { m_editorMsg = L"Save failed!"; return; }
        for (int r = 0; r < EDITOR_ROWS; ++r)
        {
            for (int c = 0; c < EDITOR_COLS; ++c)
                f << static_cast<int>(m_editGrid[c][r]) << " ";
            f << "\n";
        }
        f.close();
        m_editorMsg = L"Saved!";
        return;
    }

    // 返回按钮
    if (mx >= panelX && mx <= panelX + 180 && my >= panelY + 230 && my <= panelY + 270)
    {
        m_state = GameState::Menu;
        return;
    }

    // 点击地图网格
    int col = static_cast<int>(mx / TILE_SIZE);
    int row = static_cast<int>(my / TILE_SIZE);
    if (col >= 0 && col < EDITOR_COLS && row >= 0 && row < EDITOR_ROWS)
    {
        // Start/End 只能有一个
        if (m_editMode == 2) // Start
        {
            for (int r = 0; r < EDITOR_ROWS; ++r)
                for (int c = 0; c < EDITOR_COLS; ++c)
                    if (m_editGrid[c][r] == TileType::Start) m_editGrid[c][r] = TileType::Grass;
        }
        else if (m_editMode == 3) // End
        {
            for (int r = 0; r < EDITOR_ROWS; ++r)
                for (int c = 0; c < EDITOR_COLS; ++c)
                    if (m_editGrid[c][r] == TileType::End) m_editGrid[c][r] = TileType::Grass;
        }
        m_editGrid[col][row] = static_cast<TileType>(m_editMode);
        m_editorMsg.clear();
    }
}
