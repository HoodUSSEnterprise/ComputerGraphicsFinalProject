// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

void Game::buildCharCreateUI()
{
    bool zh = (LangManager::currentLangName() == "zh");

    m_charTitleCreateText.setFont(m_menuFont);
    m_charTitleCreateText.setString(LangManager::get(TextKey::CharSelect_New));
    m_charTitleCreateText.setCharacterSize(zh ? 42 : 48);
    m_charTitleCreateText.setFillColor(sf::Color(255, 215, 0));
    m_charTitleCreateText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_charTitleCreateText.getLocalBounds();
    m_charTitleCreateText.setOrigin(tb.width / 2, tb.height / 2);
    m_charTitleCreateText.setPosition(WINDOW_WIDTH / 2.0f, 120);

    m_charNameInput.setFont(m_menuFont);
    m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
    m_charNameInput.setCharacterSize(zh ? 20 : 22);
    m_charNameInput.setFillColor(sf::Color(150, 150, 170));

    // 确认创建按钮
    float confW = zh ? 140.0f : 120.0f;
    m_confirmCharBtn.setSize(sf::Vector2f(confW, 34));
    m_confirmCharBtn.setTexture(&m_buttonTex);
    m_confirmCharBtn.setOutlineColor(sf::Color(100, 180, 100));
    m_confirmCharBtn.setOutlineThickness(1);
    m_confirmCharBtnLabel.setFont(m_menuFont);
    m_confirmCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_Confirm));
    m_confirmCharBtnLabel.setCharacterSize(zh ? 14 : 16);
    m_confirmCharBtnLabel.setFillColor(sf::Color::White);
}

void Game::renderCharCreate()
{
    drawBackground();
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    m_window.draw(bg);

    // 标题
    sf::Text title;
    title.setFont(m_menuFont);
    title.setString(LangManager::get(TextKey::CharSelect_New));
    title.setCharacterSize(44);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2);
    title.setPosition(WINDOW_WIDTH / 2.0f, 80);
    m_window.draw(title);

    // 输入框（复用 m_charNameInput 和相关变量）
    bool zh = (LangManager::currentLangName() == "zh");
    sf::RectangleShape inputBox(sf::Vector2f(zh ? 320.0f : 300.0f, 36));
    inputBox.setPosition(WINDOW_WIDTH / 2.0f - (zh ? 160.0f : 150.0f), 200);
    inputBox.setFillColor(sf::Color(30, 30, 45));
    inputBox.setOutlineColor(sf::Color(255, 215, 0));
    inputBox.setOutlineThickness(2);
    m_window.draw(inputBox);

    m_charNameInput.setPosition(WINDOW_WIDTH / 2.0f - (zh ? 150.0f : 140.0f), 205);
    m_window.draw(m_charNameInput);

    // 确认按钮（复用 m_confirmCharBtn）
    float confW = zh ? 140.0f : 120.0f;
    m_confirmCharBtn.setSize(sf::Vector2f(confW, 34));
    m_confirmCharBtn.setPosition(WINDOW_WIDTH / 2.0f - confW / 2, 270);
    m_confirmCharBtn.setTexture(&m_buttonTex);
    m_confirmCharBtn.setOutlineColor(sf::Color(100, 180, 100));
    m_confirmCharBtn.setOutlineThickness(1);
    m_window.draw(m_confirmCharBtn);

    sf::FloatRect clb = m_confirmCharBtnLabel.getLocalBounds();
    m_confirmCharBtnLabel.setOrigin(clb.width / 2, clb.height / 2);
    m_confirmCharBtnLabel.setPosition(m_confirmCharBtn.getPosition().x + confW / 2,
                                      m_confirmCharBtn.getPosition().y + 17);
    m_window.draw(m_confirmCharBtnLabel);

    // ESC 提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 30);
    m_window.draw(hint);
}

void Game::processCharCreateEvents(const sf::Event &event)
{
    // 文字输入处理（从 processCharSelectEvents 复制过来）
    if (event.type == sf::Event::TextEntered)
    {
        sf::Uint32 ch = event.text.unicode;
        if (ch == '\b')
        {
            if (!m_newCharName.empty())
                m_newCharName.pop_back();
        }
        else if (ch == '\r' || ch == '\n')
        {
            if (!m_newCharName.empty())
            {
                m_playerData = PlayerData();
                m_playerData.name = m_newCharName;
                m_playerData.unlockedLevels = 1;
                m_playerData.save(PlayerData::makeSavePath(m_newCharName));
                m_hasCharacter = true;
                m_newCharName.clear();
                m_charNameInput.setFillColor(sf::Color(150, 150, 170));
                m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
                refreshCharList();
                initMenu();
                m_state = GameState::Menu;
            }
        }
        else if (ch >= 32 && ch < 127 && m_newCharName.size() < 16)
        {
            m_newCharName += static_cast<char>(ch);
        }

        std::wstring display = std::wstring(m_newCharName.begin(), m_newCharName.end());
        if (display.empty())
            display = LangManager::get(TextKey::CharSelect_EnterName);
        else
            display += L"_";
        m_charNameInput.setString(display);
        return;
    }

    // ESC 取消输入
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        m_newCharName.clear();
        m_charNameInput.setFillColor(sf::Color(150, 150, 170));
        m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
        m_state = GameState::CharSelect;
        return;
    }

    // 鼠标悬停效果
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;

        bool hoverConfirm = m_confirmCharBtn.getGlobalBounds().contains(mx, my);
        m_confirmCharBtn.setFillColor(hoverConfirm ? sf::Color(60, 140, 60) : sf::Color(40, 100, 40));
        return;
    }

    // 点击确认按钮
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        float mx = worldPos.x, my = worldPos.y;

        if (m_confirmCharBtn.getGlobalBounds().contains(mx, my) && !m_newCharName.empty())
        {
            m_playerData = PlayerData();
            m_playerData.name = m_newCharName;
            m_playerData.unlockedLevels = 1;
            m_playerData.save(PlayerData::makeSavePath(m_newCharName));
            m_hasCharacter = true;
            m_newCharName.clear();
            m_charNameInput.setFillColor(sf::Color(150, 150, 170));
            m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
            refreshCharList();
            initMenu();
            m_state = GameState::Menu;
        }
        return;
    }
}