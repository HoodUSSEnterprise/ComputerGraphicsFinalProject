// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

void Game::drawBackground()
{
    if (!m_bgSprites.empty() && m_bgIndex < static_cast<int>(m_bgSprites.size()))
        m_window.draw(m_bgSprites[m_bgIndex]);
}

// ============================================================
//  菜单画面
// ============================================================

void Game::initMenu()
{
    loadMenuFont();

    bool zh = (LangManager::currentLangName() == "zh");

    m_titleText.setFont(m_menuFont);
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_titleText.setCharacterSize(zh ? 56 : 60);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, zh ? 130 : 140);

    m_subtitleText.setFont(m_menuFont);
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_subtitleText.setCharacterSize(zh ? 16 : 18);
    m_subtitleText.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect sb = m_subtitleText.getLocalBounds();
    m_subtitleText.setOrigin(sb.width / 2, sb.height / 2);
    m_subtitleText.setPosition(WINDOW_WIDTH / 2.0f, zh ? 200 : 210);

    // 按钮布局: 继续游戏/新游戏/商店/自定义/设置/退出
    TextKey btnKeys[] = {TextKey::ContinueGame, TextKey::NewGame, TextKey::Shop_Title,
                         TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    float bw = zh ? 320.0f : 280.0f;   // 中文按钮更宽
    float startY = zh ? 260.0f : 275.0f;
    float gap = zh ? 70.0f : 72.0f;
    // 按钮颜色：绿/蓝/金/紫/灰/红
    sf::Color btnColors[] = {
        sf::Color(180, 255, 180),  // 继续游戏 - 绿
        sf::Color(180, 200, 255),  // 新游戏   - 蓝
        sf::Color(255, 230, 150),  // 商店     - 金
        sf::Color(220, 180, 255),  // 自定义   - 紫
        sf::Color(200, 200, 200),  // 设置     - 灰
        sf::Color(255, 160, 160),  // 退出     - 红
    };
    m_menuButtons.clear();
    for (int i = 0; i < 6; ++i)
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(bw, 52));
        btn.bg.setOrigin(bw / 2, 26);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, startY + i * gap);
        btn.bg.setTexture(&m_buttonTex);
        btn.bg.setFillColor(btnColors[i]);
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(btnKeys[i]));
        btn.label.setCharacterSize(zh ? 24 : 22);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, startY + i * gap - 4);
        btn.hovered = false;
        m_menuButtons.push_back(btn);
    }
}

void Game::loadMenuFont()
{
    std::string lang = LangManager::currentLangName();
    std::vector<std::string> paths;
    if (lang == "zh")
        paths = {"fonts/simhei.ttf", "fonts/arial.ttf"};
    else
        paths = {"fonts/arial.ttf"};

    bool loaded = false;
    for (const auto &p : paths)
        if (m_menuFont.loadFromFile(p))
        {
            std::cout << "[Font] menu loaded: " << p << std::endl;
            loaded = true;
            break;
        }
    if (!loaded)
        std::cerr << "[Font] WARNING: menu font failed to load!" << std::endl;

    m_titleText.setFont(m_menuFont);
    m_subtitleText.setFont(m_menuFont);
    for (auto &btn : m_menuButtons)
        btn.label.setFont(m_menuFont);

    // 角色选择界面
    m_charTitleText.setFont(m_menuFont);
    m_charNameInput.setFont(m_menuFont);
    m_charHintText.setFont(m_menuFont);
    m_newCharBtnLabel.setFont(m_menuFont);
    m_loadCharBtnLabel.setFont(m_menuFont);
    m_confirmCharBtnLabel.setFont(m_menuFont);
    for (auto &cb : m_charButtons)
    {
        cb.nameText.setFont(m_menuFont);
        cb.infoText.setFont(m_menuFont);
    }

    // 商店界面
    m_shopTitleText.setFont(m_menuFont);
    m_shopGoldText.setFont(m_menuFont);
    m_shopBackHint.setFont(m_menuFont);
    for (auto &sb : m_shopButtons)
    {
        sb.nameText.setFont(m_menuFont);
        sb.levelText.setFont(m_menuFont);
        sb.costText.setFont(m_menuFont);
        sb.buyLabel.setFont(m_menuFont);
    }

    // 确认对话框
    m_confirmTitleText.setFont(m_menuFont);
    m_confirmMsgText.setFont(m_menuFont);
    m_confirmYesLabel.setFont(m_menuFont);
    m_confirmNoLabel.setFont(m_menuFont);

    // 作弊消息
    m_cheatMsgText.setFont(m_menuFont);

    // 暂停菜单
    m_pauseTitle.setFont(m_menuFont);
    for (auto &btn : m_pauseButtons) btn.label.setFont(m_menuFont);
}

void Game::updateMenuHover(float mx, float my)
{
    for (auto &btn : m_menuButtons)
    {
        bool inside = btn.bg.getGlobalBounds().contains(mx, my);
        btn.hovered = inside;
        btn.bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
        btn.label.setFillColor(inside ? sf::Color(255, 215, 0) : sf::Color::White);
    }
}

int Game::getMenuButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_menuButtons.size(); ++i)
        if (m_menuButtons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    return -1;
}

void Game::renderMenu()
{
    drawBackground();
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    m_window.draw(bg);
    for (int i = 0; i < 10; ++i)
    {
        sf::RectangleShape line(sf::Vector2f(WINDOW_WIDTH, 2));
        line.setFillColor(sf::Color(30, 30, 50));
        line.setPosition(0, i * 80.0f);
        m_window.draw(line);
    }

    // 显示当前角色名
    if (m_hasCharacter)
    {
        sf::Text charText;
        charText.setFont(m_menuFont);
        std::string cn = m_playerData.name;
        charText.setString(std::wstring(cn.begin(), cn.end()) + L"  |  " +
                           std::wstring(LangManager::get(TextKey::CharSelect_Gold)) + L": " +
                           std::to_wstring(m_playerData.totalGold));
        charText.setCharacterSize(16);
        charText.setFillColor(sf::Color(100, 200, 100));
        sf::FloatRect cb = charText.getLocalBounds();
        charText.setOrigin(cb.width / 2, cb.height / 2);
        charText.setPosition(WINDOW_WIDTH / 2.0f, 80);
        m_window.draw(charText);
    }

    m_window.draw(m_titleText);
    m_window.draw(m_subtitleText);
    for (const auto &btn : m_menuButtons)
    {
        m_window.draw(btn.bg);
        m_window.draw(btn.label);
    }

    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(LangManager::get(TextKey::MenuHint));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);

    // 确认对话框
    if (m_showConfirm)
        renderConfirmDialog();
}

void Game::processMenuEvents(const sf::Event &event)
{
    // 确认对话框优先处理
    if (m_showConfirm)
    {
        if (event.type == sf::Event::MouseMoved)
        {
            sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            float mx = worldPos.x, my = worldPos.y;
            bool hoverYes = m_confirmYesBtn.getGlobalBounds().contains(mx, my);
            bool hoverNo = m_confirmNoBtn.getGlobalBounds().contains(mx, my);
            m_confirmYesBtn.setOutlineColor(hoverYes ? sf::Color(255, 80, 40) : sf::Color(255, 100, 100));
            m_confirmYesBtn.setFillColor(hoverYes ? sf::Color(255, 200, 200) : sf::Color(255, 180, 180));
            m_confirmNoBtn.setOutlineColor(hoverNo ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
            m_confirmNoBtn.setFillColor(hoverNo ? sf::Color(220, 220, 255) : sf::Color(200, 200, 220));
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            float mx = worldPos.x, my = worldPos.y;
            if (m_confirmYesBtn.getGlobalBounds().contains(mx, my))
            {
                // 确认：重置存档，从第1关开始
                m_playerData.unlockedLevels = 1;
                m_playerData.totalGold = 0;
                for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
                    m_playerData.shopLevels[i] = 0;
                m_playerData.save(PlayerData::makeSavePath(m_playerData.name));
                m_state = GameState::CampaignSelect;
                m_showConfirm = false;
            }
            else if (m_confirmNoBtn.getGlobalBounds().contains(mx, my))
            {
                m_showConfirm = false;
            }
            return;
        }
        return; // 对话框显示时忽略其他菜单事件
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        updateMenuHover(worldPos.x, worldPos.y);
        return;
    }
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    int idx = getMenuButtonIndex(worldPos.x, worldPos.y);
    switch (idx)
    {
    case 0: // 继续游戏 → 加载存档，进入战役选关
        if (m_hasCharacter)
        {
            // 重新加载最新存档
            m_playerData.load(PlayerData::makeSavePath(m_playerData.name));
            m_state = GameState::CampaignSelect;
        }
        break;
    case 1: // 新游戏 → 显示确认对话框
        if (m_hasCharacter)
            m_showConfirm = true;
        else
            m_state = GameState::CampaignSelect;
        break;
    case 2: // 商店
        if (m_hasCharacter)
        {
            buildShopUI();
            m_state = GameState::Shop;
        }
        break;
    case 3:
        m_state = GameState::CustomSetup;
        break;
    case 4:
        m_state = GameState::Settings;
        buildSettingsUI();
        break;
    case 5:
        m_window.close();
        break;
    default:
        break;
    }
}

// ============================================================
//  文本刷新
// ============================================================

void Game::refreshAllTexts()
{
    // 完全重建所有 UI（使用新语言文本和字体）
    initMenu();
    buildCharSelectUI();
    refreshCharList();
    buildConfirmUI();
    buildPauseMenu();
    buildSettingsUI();

    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_campaignScreen.refreshTexts();
    m_campaignScreen.reloadFont();
    m_customScreen.refreshTexts();
    m_customScreen.reloadFont();
}

// ============================================================
//  战役 & 自定义 委托
// ============================================================

void Game::processCampaignEvents(const sf::Event &event)
{
    LevelConfig chosen;
    int unlocked = m_unlockAll ? static_cast<int>(getCampaignLevels().size()) : m_playerData.unlockedLevels;
    if (m_campaignScreen.update(event, m_window, chosen, unlocked))
        newGame(chosen);
}

void Game::renderCampaign()
{
    int unlocked = m_unlockAll ? static_cast<int>(getCampaignLevels().size()) : m_playerData.unlockedLevels;
    m_campaignScreen.draw(m_window, unlocked);
}

void Game::processCustomSetupEvents(const sf::Event &event)
{
    CustomParams params;
    int result = m_customScreen.update(event, m_window, params);
    if (result == 1)
    {
        LevelConfig cfg;
        cfg.mapFile = "assets/maps/grassland/1-1.txt";
        cfg.biome = Biome::Grassland;
        cfg.startGold = params.startGold;
        cfg.startLives = params.startLives;
        cfg.waveCount = params.waves;
        cfg.baseEnemies = params.enemiesPerWave;
        cfg.speedMul = params.speedMul;
        cfg.hpMul = params.hpMul;
        cfg.id = "custom";
        newGame(cfg);
    }
    else if (result == 2)
    {
        m_state = GameState::Menu;
    }
}

void Game::renderCustomSetup() { m_customScreen.draw(m_window); }
