// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

// ============================================================
//  设置画面
// ============================================================

void Game::initSettings()
{
    m_volTrack.setSize(sf::Vector2f(300, 8));
    m_volTrack.setFillColor(sf::Color(60, 60, 80));
    m_volTrack.setOutlineColor(sf::Color(100, 100, 140));
    m_volTrack.setOutlineThickness(1);
    m_volKnob.setRadius(12);
    m_volKnob.setOrigin(12, 12);
    m_volKnob.setFillColor(sf::Color(255, 215, 0));
    m_bgmLabel.setFont(m_menuFont);
    m_bgmLabel.setCharacterSize(28);
    m_bgmLabel.setFillColor(sf::Color::White);
}

void Game::applyVolume() { m_bgm.setVolume(m_volume); }

int Game::getSettingsButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_settingsButtons.size(); ++i)
        if (m_settingsButtons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    return -1;
}

void Game::buildSettingsUI()
{
    m_settingsButtons.clear();
    auto makeArrow = [&](float x, float y, const std::string &label)
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(50, 50));
        btn.bg.setOrigin(25, 25);
        btn.bg.setPosition(x, y);
        btn.bg.setTexture(label == "<" ? &m_arrowLeftTex : &m_arrowRightTex);
        btn.bg.setOutlineThickness(0);
        btn.label.setFont(m_menuFont);
        btn.label.setString("");
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    };
    makeArrow(WINDOW_WIDTH / 2.0f - 180, 300, "<");
    makeArrow(WINDOW_WIDTH / 2.0f + 180, 300, ">");

    m_langLabel.setFont(m_menuFont);
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langLabel.setCharacterSize(24);
    m_langLabel.setFillColor(sf::Color(180, 180, 200));
    m_langValue.setFont(m_menuFont);
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    m_langValue.setCharacterSize(22);
    m_langValue.setFillColor(sf::Color::Yellow);

    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(280, 56));
        btn.bg.setOrigin(140, 28);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 470);
        btn.bg.setTexture(&m_buttonTex);
        btn.bg.setFillColor(sf::Color(200, 220, 255));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        btn.label.setCharacterSize(24);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 466);
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(200, 50));
        btn.bg.setOrigin(100, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 580);
        btn.bg.setTexture(&m_buttonTex);
        btn.bg.setFillColor(sf::Color(200, 200, 200));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(TextKey::Back));
        btn.label.setCharacterSize(22);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 576);
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
}

void Game::renderSettings()
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30));
    m_window.draw(bg);
    for (int i = 0; i < 10; ++i)
    {
        sf::RectangleShape line(sf::Vector2f(WINDOW_WIDTH, 2));
        line.setFillColor(sf::Color(30, 30, 50));
        line.setPosition(0, i * 80.0f);
        m_window.draw(line);
    }
    sf::Text title;
    title.setFont(m_menuFont);
    title.setString(LangManager::get(TextKey::Settings));
    title.setCharacterSize(50);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2);
    title.setPosition(WINDOW_WIDTH / 2.0f, 120);
    m_window.draw(title);
    m_langLabel.setPosition(WINDOW_WIDTH / 2.0f - 130, 280);
    m_langValue.setPosition(WINDOW_WIDTH / 2.0f - 30, 280);
    m_window.draw(m_langLabel);
    m_window.draw(m_langValue);

    float trackX = WINDOW_WIDTH / 2.0f - 150, trackY = 390;
    m_volTrack.setPosition(trackX, trackY);
    m_window.draw(m_volTrack);
    float knobX = trackX + (m_volume / 100.0f) * m_volTrack.getSize().x;
    m_volKnob.setPosition(knobX, trackY + m_volTrack.getSize().y / 2.0f);
    m_window.draw(m_volKnob);

    sf::Text volLabel;
    volLabel.setFont(m_menuFont);
    volLabel.setString(std::wstring(LangManager::get(TextKey::Volume)) + L": " + std::to_wstring(static_cast<int>(m_volume)) + L"%");
    volLabel.setCharacterSize(20);
    volLabel.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect vl = volLabel.getLocalBounds();
    volLabel.setOrigin(vl.width / 2, vl.height / 2);
    volLabel.setPosition(WINDOW_WIDTH / 2.0f, 360);
    m_window.draw(volLabel);
    for (const auto &btn : m_settingsButtons)
    {
        m_window.draw(btn.bg);
        m_window.draw(btn.label);
    }
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString("ESC: Back to Menu");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);
}

void Game::processSettingsEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (auto &btn : m_settingsButtons)
        {
            bool inside = btn.bg.getGlobalBounds().contains(worldPos.x, worldPos.y);
            btn.hovered = inside;
            btn.bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
            btn.bg.setFillColor(inside ? sf::Color(255, 255, 240) : sf::Color(200, 200, 220));
            btn.label.setFillColor(inside ? sf::Color(255, 215, 0) : sf::Color::White);
        }
        if (m_draggingVol)
        {
            float nx = std::max(m_volTrack.getPosition().x, std::min(worldPos.x, m_volTrack.getPosition().x + m_volTrack.getSize().x));
            m_volume = (nx - m_volTrack.getPosition().x) / m_volTrack.getSize().x * 100.0f;
            m_volKnob.setPosition(nx, m_volKnob.getPosition().y);
            applyVolume();
        }
        return;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        if (m_volKnob.getGlobalBounds().contains(worldPos.x, worldPos.y))
        {
            m_draggingVol = true;
            return;
        }
        int idx = getSettingsButtonIndex(worldPos.x, worldPos.y);
        switch (idx)
        {
        case 0:
        case 1:
        {
            static const char *langs[] = {"assets/lang_en.json", "assets/lang_zh.json"};
            static int cur = 0;
            cur = (idx == 0) ? (cur - 1 + 2) % 2 : (cur + 1) % 2;
            LangManager::loadLanguage(langs[cur]);
            loadMenuFont();
            m_ui.reloadFont();
            refreshAllTexts();
            break;
        }
        case 2:
            m_bgmOn = !m_bgmOn;
            if (m_bgmOn)
            {
                if (m_bgm.getStatus() != sf::Music::Playing)
                    m_bgm.play();
            }
            else
                m_bgm.pause();
            m_bgmLabel.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
            break;
        case 3:
            m_state = m_stateBeforeSettings;
            m_stateBeforeSettings = GameState::Menu;
            break;
        default:
            break;
        }
    }
    if (event.type == sf::Event::MouseButtonReleased)
        m_draggingVol = false;
}
