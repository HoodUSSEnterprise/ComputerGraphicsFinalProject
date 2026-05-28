// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

// ============================================================
//  加载角色界面（独立屏幕）
// ============================================================

void Game::renderCharLoad()
{
    drawBackground();
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    m_window.draw(bg);

    // 标题
    sf::Text title;
    title.setFont(m_menuFont);
    title.setString(LangManager::get(TextKey::LoadGame));
    title.setCharacterSize(44);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2);
    title.setPosition(WINDOW_WIDTH / 2.0f, 80);
    m_window.draw(title);

    // 角色列表
    for (auto &cb : m_charButtons)
    {
        m_window.draw(cb.bg);
        m_window.draw(cb.nameText);
        m_window.draw(cb.infoText);
    }

    if (m_charList.empty())
    {
        m_charHintText.setString(LangManager::get(TextKey::CharSelect_Empty));
        sf::FloatRect hb = m_charHintText.getLocalBounds();
        m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
        m_charHintText.setPosition(WINDOW_WIDTH / 2.0f, 350);
        m_window.draw(m_charHintText);
    }

    // 返回提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);
}

void Game::processCharLoadEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;
        for (size_t i = 0; i < m_charButtons.size(); ++i)
        {
            bool inside = m_charButtons[i].bg.getGlobalBounds().contains(mx, my);
            m_charButtons[i].hovered = inside;
            m_charButtons[i].bg.setFillColor(inside ? sf::Color(255, 255, 255, 40) : sf::Color(255, 255, 255, 0));
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    for (size_t i = 0; i < m_charButtons.size(); ++i)
    {
        if (m_charButtons[i].bg.getGlobalBounds().contains(mx, my))
        {
            m_playerData = m_charList[i];
            m_hasCharacter = true;
            m_playerData.save(PlayerData::makeSavePath(m_playerData.name));
            initMenu();
            m_state = GameState::Menu;
            return;
        }
    }
}
