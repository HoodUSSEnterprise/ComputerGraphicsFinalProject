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

    m_charHintText.setFont(m_menuFont);
    m_charHintText.setCharacterSize(14);
    m_charHintText.setFillColor(sf::Color(120, 120, 140));

    // 新建角色按钮
    float btnW = zh ? 380.0f : 340.0f;
    m_newCharBtn.setSize(sf::Vector2f(btnW, 80));
    m_newCharBtn.setPosition(WINDOW_WIDTH / 2.0f - btnW / 2, 250); // 调整位置，两个按钮居中
    m_newCharBtn.setTexture(&m_buttonTex);
    m_newCharBtn.setOutlineColor(sf::Color(100, 180, 100));
    m_newCharBtn.setOutlineThickness(2);
    m_newCharBtnLabel.setFont(m_menuFont);
    m_newCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_New));
    m_newCharBtnLabel.setCharacterSize(zh ? 36 : 40);
    m_newCharBtnLabel.setFillColor(sf::Color::White);

    // 加载角色按钮
    m_loadCharBtn.setSize(sf::Vector2f(btnW, 80));
    m_loadCharBtn.setPosition(WINDOW_WIDTH / 2.0f - btnW / 2, 360);
    m_loadCharBtn.setTexture(&m_buttonTex);
    m_loadCharBtn.setOutlineColor(sf::Color(100, 150, 200));
    m_loadCharBtn.setOutlineThickness(2);
    m_loadCharBtnLabel.setFont(m_menuFont);
    m_loadCharBtnLabel.setString(LangManager::get(TextKey::LoadGame));
    m_loadCharBtnLabel.setCharacterSize(zh ? 36 : 40);
    m_loadCharBtnLabel.setFillColor(sf::Color::White);
}

void Game::refreshCharList()
{
    m_charList = PlayerData::listCharacters("saves");
    m_charButtons.clear();

    float y = 160.0f;
    for (size_t i = 0; i < m_charList.size(); ++i)
    {
        CharButton cb;
        // 大卡片背景（information.png）
        cb.bg.setSize(sf::Vector2f(560, 60));
        cb.bg.setPosition(WINDOW_WIDTH / 2.0f - 280, y);
        cb.bg.setTexture(&m_infoTex);
        cb.bg.setOutlineThickness(0);

        std::string nm = m_charList[i].name;
        cb.nameText.setFont(m_menuFont);
        cb.nameText.setString(std::wstring(nm.begin(), nm.end()));
        cb.nameText.setCharacterSize(20);
        cb.nameText.setFillColor(sf::Color(255, 215, 0));
        cb.nameText.setPosition(WINDOW_WIDTH / 2.0f - 265, y + 6);

        std::wstring info = std::to_wstring(m_charList[i].unlockedLevels) + L"/11 " +
                            LangManager::get(TextKey::CharSelect_Progress) +
                            L"  |  " + std::to_wstring(m_charList[i].totalGold) + L" " +
                            LangManager::get(TextKey::CharSelect_Gold);
        cb.infoText.setFont(m_menuFont);
        cb.infoText.setString(info);
        cb.infoText.setCharacterSize(15);
        cb.infoText.setFillColor(sf::Color(160, 180, 200));
        cb.infoText.setPosition(WINDOW_WIDTH / 2.0f - 265, y + 32);

        m_charButtons.push_back(cb);
        y += 72.0f;
    }

    m_charHintText.setString(
        m_charList.empty()
            ? LangManager::get(TextKey::CharSelect_Empty)
            : L"");
    sf::FloatRect hb = m_charHintText.getLocalBounds();
    m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
    m_charHintText.setPosition(WINDOW_WIDTH / 2.0f,
                               m_charList.empty() ? 350.0f : y + 20);
}

void Game::renderCharSelect()
{
    drawBackground();
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    m_window.draw(bg);

    m_window.draw(m_charTitleText);

    // 新建角色按钮
    m_window.draw(m_newCharBtn);
    sf::FloatRect nlb = m_newCharBtnLabel.getLocalBounds();
    m_newCharBtnLabel.setOrigin(nlb.width / 2, nlb.height / 2);
    m_newCharBtnLabel.setPosition(m_newCharBtn.getPosition().x + m_newCharBtn.getSize().x / 2,
                                  m_newCharBtn.getPosition().y + m_newCharBtn.getSize().y / 2);
    m_window.draw(m_newCharBtnLabel);

    // 加载角色按钮
    m_window.draw(m_loadCharBtn);
    sf::FloatRect lclb = m_loadCharBtnLabel.getLocalBounds();
    m_loadCharBtnLabel.setOrigin(lclb.width / 2, lclb.height / 2);
    m_loadCharBtnLabel.setPosition(m_loadCharBtn.getPosition().x + m_loadCharBtn.getSize().x / 2,
                                   m_loadCharBtn.getPosition().y + m_loadCharBtn.getSize().y / 2);
    m_window.draw(m_loadCharBtnLabel);

    // ESC 提示
    if (!m_charList.empty())
    {
        m_charHintText.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
        sf::FloatRect hb = m_charHintText.getLocalBounds();
        m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
        m_charHintText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 30); // 放在底部可见位置
    }
    else
    {
        m_charHintText.setString(LangManager::get(TextKey::CharSelect_Empty));
        sf::FloatRect hb = m_charHintText.getLocalBounds();
        m_charHintText.setOrigin(hb.width / 2, hb.height / 2);
        m_charHintText.setPosition(WINDOW_WIDTH / 2.0f, 350);
    }
    m_window.draw(m_charHintText);
}

void Game::processCharSelectEvents(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_window.close();
        }
        return;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;

        // 新建按钮 hover
        bool hoverNew = m_newCharBtn.getGlobalBounds().contains(mx, my);
        m_newCharBtn.setOutlineColor(hoverNew ? sf::Color(255, 215, 0) : sf::Color(100, 180, 100));

        // 加载按钮 hover
        bool hoverLoad = m_loadCharBtn.getGlobalBounds().contains(mx, my);
        m_loadCharBtn.setOutlineColor(hoverLoad ? sf::Color(255, 215, 0) : sf::Color(100, 150, 200));

        // 角色列表 hover（仅在显示列表时）
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

    // 点击新建按钮 → 切换到独立的新建角色页面
    if (m_newCharBtn.getGlobalBounds().contains(mx, my))
    {
        m_newCharName.clear();
        m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
        m_state = GameState::CharCreate; // 切换到新建角色页面
        return;
    }

    // 点击加载角色按钮 → 进入独立加载角色界面
    if (m_loadCharBtn.getGlobalBounds().contains(mx, my))
    {
        loadMenuFont(); // 确保字体正确加载
        refreshCharList();
        m_state = GameState::CharLoad;
        return;
    }

    // 点击已有角色 → 加载并进入菜单
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