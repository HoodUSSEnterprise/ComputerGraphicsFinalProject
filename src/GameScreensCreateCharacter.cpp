// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

// ============================================================
//  角色选择界面
// ============================================================

void Game::buildCharSelectUI()
{
    bool zh = (LangManager::currentLangName() == "zh");

    m_charTitleText.setFont(m_menuFont);
    m_charTitleText.setString(LangManager::get(TextKey::CharSelect_Title));
    m_charTitleText.setCharacterSize(zh ? 42 : 48);
    m_charTitleText.setFillColor(sf::Color(255, 215, 0));
    m_charTitleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_charTitleText.getLocalBounds();
    m_charTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_charTitleText.setPosition(WINDOW_WIDTH / 2.0f, 120);

    m_charNameInput.setFont(m_menuFont);
    m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
    m_charNameInput.setCharacterSize(zh ? 20 : 22);
    m_charNameInput.setFillColor(sf::Color(150, 150, 170));

    m_charHintText.setFont(m_menuFont);
    m_charHintText.setCharacterSize(14);
    m_charHintText.setFillColor(sf::Color(120, 120, 140));

    // 新建角色按钮
    float btnW = zh ? 380.0f : 340.0f;
    m_newCharBtn.setSize(sf::Vector2f(btnW, 44));
    m_newCharBtn.setPosition(WINDOW_WIDTH / 2.0f - btnW / 2, 200);
    m_newCharBtn.setFillColor(sf::Color(40, 80, 40));
    m_newCharBtn.setOutlineColor(sf::Color(100, 180, 100));
    m_newCharBtn.setOutlineThickness(2);
    m_newCharBtnLabel.setFont(m_menuFont);
    m_newCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_New));
    m_newCharBtnLabel.setCharacterSize(zh ? 18 : 20);
    m_newCharBtnLabel.setFillColor(sf::Color::White);

    // 确认创建按钮
    float confW = zh ? 140.0f : 120.0f;
    m_confirmCharBtn.setSize(sf::Vector2f(confW, 34));
    m_confirmCharBtn.setFillColor(sf::Color(40, 100, 40));
    m_confirmCharBtn.setOutlineColor(sf::Color(100, 200, 100));
    m_confirmCharBtn.setOutlineThickness(1);
    m_confirmCharBtnLabel.setFont(m_menuFont);
    m_confirmCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_Confirm));
    m_confirmCharBtnLabel.setCharacterSize(zh ? 14 : 16);
    m_confirmCharBtnLabel.setFillColor(sf::Color::White);
}

void Game::refreshCharList()
{
    m_charList = PlayerData::listCharacters("saves");
    m_charButtons.clear();

    float y = 280.0f;
    for (size_t i = 0; i < m_charList.size(); ++i)
    {
        CharButton cb;
        cb.bg.setSize(sf::Vector2f(400, 50));
        cb.bg.setPosition(WINDOW_WIDTH / 2.0f - 200, y);
        cb.bg.setFillColor(sf::Color(35, 35, 55));
        cb.bg.setOutlineColor(sf::Color(70, 70, 100));
        cb.bg.setOutlineThickness(1);

        std::string nm = m_charList[i].name;
        cb.nameText.setFont(m_menuFont);
        cb.nameText.setString(std::wstring(nm.begin(), nm.end()));
        cb.nameText.setCharacterSize(18);
        cb.nameText.setFillColor(sf::Color(255, 215, 0));
        cb.nameText.setPosition(WINDOW_WIDTH / 2.0f - 190, y + 4);

        std::string info = std::to_string(m_charList[i].unlockedLevels) + "/11 " +
                           std::string(LangManager::get(TextKey::CharSelect_Progress).begin(),
                                       LangManager::get(TextKey::CharSelect_Progress).end()) +
                           "  |  " + std::to_string(m_charList[i].totalGold) + " " +
                           std::string(LangManager::get(TextKey::CharSelect_Gold).begin(),
                                       LangManager::get(TextKey::CharSelect_Gold).end());
        cb.infoText.setFont(m_menuFont);
        cb.infoText.setString(std::wstring(info.begin(), info.end()));
        cb.infoText.setCharacterSize(14);
        cb.infoText.setFillColor(sf::Color(180, 180, 200));
        cb.infoText.setPosition(WINDOW_WIDTH / 2.0f - 190, y + 26);

        m_charButtons.push_back(cb);
        y += 56.0f;
    }

    m_charHintText.setString(
        m_charList.empty()
            ? LangManager::get(TextKey::CharSelect_Empty)
            : L"");
    sf::FloatRect hb = m_charHintText.getLocalBounds();
    m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
    m_charHintText.setPosition(WINDOW_WIDTH / 2.0f,
                               m_charList.empty() ? 400.0f : y + 20);
}

void Game::renderCharSelect()
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30));
    m_window.draw(bg);

    m_window.draw(m_charTitleText);

    // 新建角色区域
    bool isCreating = !m_newCharName.empty() || m_charNameInput.getString().getSize() > 0;
    // 判断是否处于输入名字模式
    bool inputMode = (m_charNameInput.getFillColor() == sf::Color::White);

    m_window.draw(m_newCharBtn);
    sf::FloatRect nlb = m_newCharBtnLabel.getLocalBounds();
    m_newCharBtnLabel.setOrigin(nlb.width / 2, nlb.height / 2);
    m_newCharBtnLabel.setPosition(m_newCharBtn.getPosition().x + m_newCharBtn.getSize().x / 2,
                                  m_newCharBtn.getPosition().y + m_newCharBtn.getSize().y / 2);
    m_window.draw(m_newCharBtnLabel);

    // 名字输入框
    if (inputMode)
    {
        bool zh = (LangManager::currentLangName() == "zh");
        sf::RectangleShape inputBox(sf::Vector2f(zh ? 320.0f : 300.0f, 36));
        inputBox.setPosition(WINDOW_WIDTH / 2.0f - (zh ? 160.0f : 150.0f), 210);
        inputBox.setFillColor(sf::Color(30, 30, 45));
        inputBox.setOutlineColor(sf::Color(255, 215, 0));
        inputBox.setOutlineThickness(2);
        m_window.draw(inputBox);

        m_charNameInput.setPosition(WINDOW_WIDTH / 2.0f - (zh ? 150.0f : 140.0f), 215);
        m_window.draw(m_charNameInput);

        m_confirmCharBtn.setPosition(WINDOW_WIDTH / 2.0f + (zh ? 170.0f : 160.0f), 212);
        sf::FloatRect clb = m_confirmCharBtnLabel.getLocalBounds();
        m_confirmCharBtnLabel.setOrigin(clb.width / 2, clb.height / 2);
        m_confirmCharBtnLabel.setPosition(m_confirmCharBtn.getPosition().x + m_confirmCharBtn.getSize().x / 2,
                                          m_confirmCharBtn.getPosition().y + m_confirmCharBtn.getSize().y / 2);
        m_window.draw(m_confirmCharBtn);
        m_window.draw(m_confirmCharBtnLabel);
    }

    // 已有角色列表
    for (auto &cb : m_charButtons)
    {
        m_window.draw(cb.bg);
        m_window.draw(cb.nameText);
        m_window.draw(cb.infoText);
    }

    if (!m_charList.empty())
    {
        m_charHintText.setString(L"ESC: Exit");
        sf::FloatRect hb = m_charHintText.getLocalBounds();
        m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
        m_charHintText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    }
    m_window.draw(m_charHintText);

    sf::Text langText;
    langText.setFont(m_menuFont);
    langText.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    langText.setCharacterSize(16);
    langText.setFillColor(sf::Color(100, 200, 100));
    sf::FloatRect lb = langText.getLocalBounds();
    langText.setOrigin(lb.width / 2, lb.height / 2);
    langText.setPosition(WINDOW_WIDTH / 2.0f, 660);
    m_window.draw(langText);
}

void Game::processCharSelectEvents(const sf::Event &event)
{
    // TextEntered 用于输入角色名
    if (event.type == sf::Event::TextEntered)
    {
        bool inputMode = (m_charNameInput.getFillColor() == sf::Color::White);
        if (!inputMode)
            return;

        sf::Uint32 ch = event.text.unicode;
        if (ch == '\b') // 退格
        {
            if (!m_newCharName.empty())
                m_newCharName.pop_back();
        }
        else if (ch == '\r' || ch == '\n')
        {
            // 回车确认创建
            if (!m_newCharName.empty())
            {
                m_playerData = PlayerData();
                m_playerData.name = m_newCharName;
                m_playerData.unlockedLevels = 1;
                m_playerData.save(PlayerData::makeSavePath(m_newCharName));
                m_hasCharacter = true;
                m_newCharName.clear();
                m_charNameInput.setFillColor(sf::Color(150, 150, 170));
                std::wstring placeholder = LangManager::get(TextKey::CharSelect_EnterName);
                m_charNameInput.setString(placeholder);
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
            display += L"_"; // 光标
        m_charNameInput.setString(display);
        return;
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            bool inputMode = (m_charNameInput.getFillColor() == sf::Color::White);
            if (inputMode)
            {
                // 取消输入
                m_newCharName.clear();
                m_charNameInput.setFillColor(sf::Color(150, 150, 170));
                m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
            }
            else
            {
                m_window.close();
            }
        }
        return;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;

        // 新建按钮 hover
        bool hoverNew = m_newCharBtn.getGlobalBounds().contains(mx, my);
        m_newCharBtn.setFillColor(hoverNew ? sf::Color(60, 120, 60) : sf::Color(40, 80, 40));

        // 确认按钮 hover
        bool inputMode = (m_charNameInput.getFillColor() == sf::Color::White);
        if (inputMode)
        {
            bool hoverConfirm = m_confirmCharBtn.getGlobalBounds().contains(mx, my);
            m_confirmCharBtn.setFillColor(hoverConfirm ? sf::Color(60, 140, 60) : sf::Color(40, 100, 40));
        }

        // 角色列表 hover
        for (size_t i = 0; i < m_charButtons.size(); ++i)
        {
            bool inside = m_charButtons[i].bg.getGlobalBounds().contains(mx, my);
            m_charButtons[i].hovered = inside;
            m_charButtons[i].bg.setFillColor(inside ? sf::Color(55, 55, 90) : sf::Color(35, 35, 55));
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    // 点击新建按钮 → 进入输入模式
    if (m_newCharBtn.getGlobalBounds().contains(mx, my))
    {
        m_charNameInput.setFillColor(sf::Color::White);
        m_newCharName.clear();
        m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
        return;
    }

    // 点击确认按钮 → 创建角色
    bool inputMode = (m_charNameInput.getFillColor() == sf::Color::White);
    if (inputMode && m_confirmCharBtn.getGlobalBounds().contains(mx, my))
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
        return;
    }

    // 点击已有角色 → 加载并进入菜单
    for (size_t i = 0; i < m_charButtons.size(); ++i)
    {
        if (m_charButtons[i].bg.getGlobalBounds().contains(mx, my))
        {
            m_playerData = m_charList[i];
            m_hasCharacter = true;
            m_playerData.save(PlayerData::makeSavePath(m_playerData.name)); // 更新存档
            initMenu();
            m_state = GameState::Menu;
            return;
        }
    }
}
