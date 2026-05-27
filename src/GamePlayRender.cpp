// GamePlay.cpp — Game 的游戏逻辑实现
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

// ============================================================
//  渲染
// ============================================================

void Game::renderPlaying()
{
    drawBackground();
    m_map.draw(m_window);
    for (const auto &tower : m_towers) tower->draw(m_window);
    for (const auto &proj : m_projectiles) proj->draw(m_window);
    for (const auto &enemy : m_enemies) enemy->draw(m_window);
    m_ui.draw(m_window);
    drawPopup();

    // 作弊码激活提示
    if (m_cheatMsgClock.getElapsedTime().asSeconds() < 3.0f)
    {
        m_window.draw(m_cheatMsgText);
    }

    // 暂停菜单
    if (m_paused)
        renderPauseMenu();
}

void Game::renderEndScreen()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // 对话框背景
    float dlgW = zh ? 460.0f : 420.0f;
    sf::RectangleShape dlgBg(sf::Vector2f(dlgW, 240));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - dlgW / 2, WINDOW_HEIGHT / 2.0f - 120);
    dlgBg.setFillColor(sf::Color(25, 25, 45));
    dlgBg.setOutlineColor(m_state == GameState::GameWon ? sf::Color(255, 215, 0) : sf::Color(255, 80, 80));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    // 标题
    sf::Text title;
    title.setFont(m_menuFont);
    title.setCharacterSize(zh ? 40 : 44);
    title.setStyle(sf::Text::Bold);
    if (m_state == GameState::GameWon)
    { title.setString(LangManager::get(TextKey::Victory)); title.setFillColor(sf::Color::Yellow); }
    else
    { title.setString(LangManager::get(TextKey::GameOver)); title.setFillColor(sf::Color::Red); }
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2);
    title.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 70);
    m_window.draw(title);

    // 通关时显示"下一关"和"返回主页"按钮
    if (m_state == GameState::GameWon && m_hasCharacter && m_currentCampaignIndex >= 0)
    {
        auto levels = getCampaignLevels();
        bool hasNext = (m_currentCampaignIndex + 1 < m_playerData.unlockedLevels &&
                        m_currentCampaignIndex + 1 < static_cast<int>(levels.size()));

        float btnW = zh ? 180.0f : 160.0f;
        float gap = zh ? 20.0f : 10.0f;

        // 下一关按钮
        {
            sf::RectangleShape btn(sf::Vector2f(btnW, 44));
            btn.setPosition(WINDOW_WIDTH / 2.0f - btnW - gap / 2, WINDOW_HEIGHT / 2.0f + 5);
            btn.setFillColor(hasNext ? sf::Color(40, 100, 40) : sf::Color(50, 50, 50));
            btn.setOutlineColor(hasNext ? sf::Color(100, 200, 100) : sf::Color(80, 80, 80));
            btn.setOutlineThickness(2);
            m_window.draw(btn);

            sf::Text btnText;
            btnText.setFont(m_menuFont);
            btnText.setString(LangManager::get(TextKey::NextLevel));
            btnText.setCharacterSize(zh ? 18 : 20);
            btnText.setFillColor(hasNext ? sf::Color::White : sf::Color(120, 120, 120));
            sf::FloatRect bb = btnText.getLocalBounds();
            btnText.setOrigin(bb.width / 2, bb.height / 2);
            btnText.setPosition(WINDOW_WIDTH / 2.0f - btnW / 2 - gap / 2, WINDOW_HEIGHT / 2.0f + 27);
            m_window.draw(btnText);
        }

        // 返回主页按钮
        {
            sf::RectangleShape btn(sf::Vector2f(btnW, 44));
            btn.setPosition(WINDOW_WIDTH / 2.0f + gap / 2, WINDOW_HEIGHT / 2.0f + 5);
            btn.setFillColor(sf::Color(50, 50, 70));
            btn.setOutlineColor(sf::Color(100, 100, 140));
            btn.setOutlineThickness(2);
            m_window.draw(btn);

            sf::Text btnText;
            btnText.setFont(m_menuFont);
            btnText.setString(LangManager::get(TextKey::BackToMenu));
            btnText.setCharacterSize(zh ? 18 : 20);
            btnText.setFillColor(sf::Color::White);
            sf::FloatRect bb = btnText.getLocalBounds();
            btnText.setOrigin(bb.width / 2, bb.height / 2);
            btnText.setPosition(WINDOW_WIDTH / 2.0f + btnW / 2 + gap / 2, WINDOW_HEIGHT / 2.0f + 27);
            m_window.draw(btnText);
        }
    }
    else if (m_state == GameState::GameOver)
    {
        sf::Text hint;
        hint.setFont(m_menuFont);
        hint.setCharacterSize(zh ? 20 : 22);
        hint.setFillColor(sf::Color::White);
        hint.setString(std::wstring(LangManager::get(TextKey::PressR)) + L"    |    " + std::wstring(LangManager::get(TextKey::EscToMenu)));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2, hb.height / 2);
        hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 30);
        m_window.draw(hint);
    }
    else
    {
        sf::Text hint;
        hint.setFont(m_menuFont);
        hint.setCharacterSize(zh ? 20 : 22);
        hint.setFillColor(sf::Color::White);
        hint.setString(LangManager::get(TextKey::BackToMenu));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2, hb.height / 2);
        hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 30);
        m_window.draw(hint);
    }
}