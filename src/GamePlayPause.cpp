// GamePlay.cpp — Game 的游戏逻辑实现
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

// ============================================================
//  暂停菜单
// ============================================================

void Game::buildPauseMenu()
{
    bool zh = (LangManager::currentLangName() == "zh");

    m_pauseTitle.setFont(m_menuFont);
    m_pauseTitle.setString(zh ? L"暂停" : L"PAUSED");
    m_pauseTitle.setCharacterSize(zh ? 44 : 48);
    m_pauseTitle.setFillColor(sf::Color(255, 215, 0));
    m_pauseTitle.setStyle(sf::Text::Bold);

    TextKey keys[] = {TextKey::ContinueGame, TextKey::Settings, TextKey::Exit};
    float bw = zh ? 300.0f : 260.0f;
    m_pauseButtons.clear();
    for (int i = 0; i < 3; ++i)
    {
        PauseButton btn;
        btn.bg.setSize(sf::Vector2f(bw, 50));
        btn.bg.setFillColor(sf::Color(40, 40, 60));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(keys[i]));
        btn.label.setCharacterSize(zh ? 22 : 24);
        btn.label.setFillColor(sf::Color::White);
        m_pauseButtons.push_back(btn);
    }
    // 退出按钮标红
    m_pauseButtons[2].bg.setFillColor(sf::Color(80, 30, 30));
    m_pauseButtons[2].bg.setOutlineColor(sf::Color(200, 80, 80));
}

void Game::renderPauseMenu()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    m_window.draw(overlay);

    // 对话框背景
    float dlgW = zh ? 380.0f : 360.0f;
    sf::RectangleShape dlgBg(sf::Vector2f(dlgW, 320));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - dlgW / 2, WINDOW_HEIGHT / 2.0f - 160);
    dlgBg.setFillColor(sf::Color(20, 20, 40));
    dlgBg.setOutlineColor(sf::Color(255, 215, 0));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    // 标题
    sf::FloatRect tb = m_pauseTitle.getLocalBounds();
    m_pauseTitle.setOrigin(tb.width / 2, tb.height / 2);
    m_pauseTitle.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 110);
    m_window.draw(m_pauseTitle);

    // 按钮
    for (int i = 0; i < 3; ++i)
    {
        auto &btn = m_pauseButtons[i];
        float bw = zh ? 300.0f : 260.0f;
        btn.bg.setOrigin(bw / 2, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 30 + i * 65.0f);
        m_window.draw(btn.bg);

        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 34 + i * 65.0f);
        m_window.draw(btn.label);
    }

    // 提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(std::wstring(L"ESC: ") + LangManager::get(TextKey::ContinueGame));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 175);
    m_window.draw(hint);
}

void Game::processPauseEvents(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_paused = false;
            return;
        }
        return;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;
        for (int i = 0; i < 3; ++i)
        {
            bool inside = m_pauseButtons[i].bg.getGlobalBounds().contains(mx, my);
            m_pauseButtons[i].hovered = inside;
            if (i == 2)
            {
                m_pauseButtons[i].bg.setFillColor(inside ? sf::Color(180, 50, 50) : sf::Color(80, 30, 30));
                m_pauseButtons[i].bg.setOutlineColor(inside ? sf::Color(255, 100, 100) : sf::Color(200, 80, 80));
            }
            else
            {
                m_pauseButtons[i].bg.setFillColor(inside ? sf::Color(60, 60, 100) : sf::Color(40, 40, 60));
                m_pauseButtons[i].bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
            }
            m_pauseButtons[i].label.setFillColor(inside ? sf::Color(255, 215, 0) : sf::Color::White);
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    for (int i = 0; i < 3; ++i)
    {
        if (m_pauseButtons[i].bg.getGlobalBounds().contains(mx, my))
        {
            switch (i)
            {
            case 0: // 继续游戏
                m_paused = false;
                break;
            case 1: // 设置
                m_paused = false;
                saveGame();
                m_stateBeforeSettings = GameState::Playing;
                m_state = GameState::Settings;
                buildSettingsUI();
                break;
            case 2: // 保存并退出
                m_paused = false;
                saveGame();
                returnToMenu();
                break;
            }
            break;
        }
    }
}
