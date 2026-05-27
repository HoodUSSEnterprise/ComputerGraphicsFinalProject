// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

// ============================================================
//  菜单画面
// ============================================================

void Game::initMenu()
{
    loadMenuFont();

    m_titleText.setFont(m_menuFont);
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_titleText.setCharacterSize(60);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 140);

    m_subtitleText.setFont(m_menuFont);
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_subtitleText.setCharacterSize(18);
    m_subtitleText.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect sb = m_subtitleText.getLocalBounds();
    m_subtitleText.setOrigin(sb.width / 2, sb.height / 2);
    m_subtitleText.setPosition(WINDOW_WIDTH / 2.0f, 210);

    // 新按钮布局: 继续游戏, 新游戏, 商店, 自定义, 设置, 退出
    TextKey btnKeys[] = {TextKey::ContinueGame, TextKey::NewGame, TextKey::Shop_Title,
                         TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    m_menuButtons.clear();
    for (int i = 0; i < 6; ++i)
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(280, 52));
        btn.bg.setOrigin(140, 26);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 285 + i * 60.0f);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(btnKeys[i]));
        btn.label.setCharacterSize(22);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 285 + i * 60.0f - 4);
        btn.hovered = false;
        m_menuButtons.push_back(btn);
    }
}

void Game::loadMenuFont()
{
    std::vector<std::string> paths = {LangManager::getFontPath()};
    std::string lang = LangManager::currentLangName();
    if (lang == "zh")
        paths.push_back("fonts/simhei.ttf");
    paths.push_back("fonts/arial.ttf");

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
        btn.bg.setFillColor(inside ? sf::Color(70, 70, 100) : sf::Color(50, 50, 70));
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

    sf::Text langText;
    langText.setFont(m_menuFont);
    langText.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    langText.setCharacterSize(16);
    langText.setFillColor(sf::Color(100, 200, 100));
    sf::FloatRect lb = langText.getLocalBounds();
    langText.setOrigin(lb.width / 2, lb.height / 2);
    langText.setPosition(WINDOW_WIDTH / 2.0f, 660);
    m_window.draw(langText);

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
            m_confirmYesBtn.setFillColor(hoverYes ? sf::Color(200, 60, 60) : sf::Color(150, 40, 40));
            m_confirmNoBtn.setFillColor(hoverNo ? sf::Color(70, 70, 100) : sf::Color(50, 50, 70));
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
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(label);
        btn.label.setCharacterSize(30);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(x, y - 4);
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
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 400);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        btn.label.setCharacterSize(24);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 396);
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(200, 50));
        btn.bg.setOrigin(100, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 520);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(TextKey::Back));
        btn.label.setCharacterSize(22);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 516);
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
    m_langLabel.setPosition(WINDOW_WIDTH / 2.0f - 130, 290);
    m_langValue.setPosition(WINDOW_WIDTH / 2.0f - 30, 290);
    m_window.draw(m_langLabel);
    m_window.draw(m_langValue);

    float trackX = WINDOW_WIDTH / 2.0f - 150, trackY = 360;
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
    volLabel.setPosition(WINDOW_WIDTH / 2.0f, 330);
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
            btn.bg.setFillColor(inside ? sf::Color(70, 70, 100) : sf::Color(50, 50, 70));
            btn.bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
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

// ============================================================
//  文本刷新
// ============================================================

void Game::refreshAllTexts()
{
    TextKey btnKeys[] = {TextKey::ContinueGame, TextKey::NewGame, TextKey::Shop_Title,
                         TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    for (size_t i = 0; i < m_menuButtons.size() && i < 6; ++i)
        m_menuButtons[i].label.setString(LangManager::get(btnKeys[i]));
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    if (!m_settingsButtons.empty())
    {
        m_settingsButtons[2].label.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        m_settingsButtons[3].label.setString(LangManager::get(TextKey::Back));
    }
    m_campaignScreen.refreshTexts();
    m_campaignScreen.reloadFont();
    m_customScreen.refreshTexts();
    m_customScreen.reloadFont();
}

// ============================================================
//  角色选择界面
// ============================================================

void Game::buildCharSelectUI()
{
    m_charTitleText.setFont(m_menuFont);
    m_charTitleText.setString(LangManager::get(TextKey::CharSelect_Title));
    m_charTitleText.setCharacterSize(48);
    m_charTitleText.setFillColor(sf::Color(255, 215, 0));
    m_charTitleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_charTitleText.getLocalBounds();
    m_charTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_charTitleText.setPosition(WINDOW_WIDTH / 2.0f, 120);

    m_charNameInput.setFont(m_menuFont);
    m_charNameInput.setString(LangManager::get(TextKey::CharSelect_EnterName));
    m_charNameInput.setCharacterSize(22);
    m_charNameInput.setFillColor(sf::Color(150, 150, 170));

    m_charHintText.setFont(m_menuFont);
    m_charHintText.setCharacterSize(14);
    m_charHintText.setFillColor(sf::Color(120, 120, 140));

    // 新建角色按钮
    m_newCharBtn.setSize(sf::Vector2f(340, 44));
    m_newCharBtn.setPosition(WINDOW_WIDTH / 2.0f - 170, 200);
    m_newCharBtn.setFillColor(sf::Color(40, 80, 40));
    m_newCharBtn.setOutlineColor(sf::Color(100, 180, 100));
    m_newCharBtn.setOutlineThickness(2);
    m_newCharBtnLabel.setFont(m_menuFont);
    m_newCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_New));
    m_newCharBtnLabel.setCharacterSize(20);
    m_newCharBtnLabel.setFillColor(sf::Color::White);

    // 确认创建按钮（默认隐藏）
    m_confirmCharBtn.setSize(sf::Vector2f(120, 34));
    m_confirmCharBtn.setFillColor(sf::Color(40, 100, 40));
    m_confirmCharBtn.setOutlineColor(sf::Color(100, 200, 100));
    m_confirmCharBtn.setOutlineThickness(1);
    m_confirmCharBtnLabel.setFont(m_menuFont);
    m_confirmCharBtnLabel.setString(LangManager::get(TextKey::CharSelect_Confirm));
    m_confirmCharBtnLabel.setCharacterSize(16);
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
    m_newCharBtnLabel.setPosition(m_newCharBtn.getPosition().x + 170, m_newCharBtn.getPosition().y + 22);
    m_window.draw(m_newCharBtnLabel);

    // 名字输入框
    if (inputMode)
    {
        sf::RectangleShape inputBox(sf::Vector2f(300, 36));
        inputBox.setPosition(WINDOW_WIDTH / 2.0f - 150, 210);
        inputBox.setFillColor(sf::Color(30, 30, 45));
        inputBox.setOutlineColor(sf::Color(255, 215, 0));
        inputBox.setOutlineThickness(2);
        m_window.draw(inputBox);

        m_charNameInput.setPosition(WINDOW_WIDTH / 2.0f - 140, 215);
        m_window.draw(m_charNameInput);

        m_confirmCharBtn.setPosition(WINDOW_WIDTH / 2.0f + 160, 212);
        sf::FloatRect clb = m_confirmCharBtnLabel.getLocalBounds();
        m_confirmCharBtnLabel.setOrigin(clb.width / 2, clb.height / 2);
        m_confirmCharBtnLabel.setPosition(m_confirmCharBtn.getPosition().x + 60, m_confirmCharBtn.getPosition().y + 17);
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

// ============================================================
//  商店界面
// ============================================================

void Game::buildShopUI()
{
    m_shopButtons.clear();

    m_shopTitleText.setFont(m_menuFont);
    m_shopTitleText.setString(LangManager::get(TextKey::Shop_Title));
    m_shopTitleText.setCharacterSize(44);
    m_shopTitleText.setFillColor(sf::Color(255, 215, 0));
    m_shopTitleText.setStyle(sf::Text::Bold);

    m_shopGoldText.setFont(m_menuFont);
    m_shopGoldText.setCharacterSize(20);
    m_shopGoldText.setFillColor(sf::Color(255, 215, 0));

    m_shopBackHint.setFont(m_menuFont);
    m_shopBackHint.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
    m_shopBackHint.setCharacterSize(14);
    m_shopBackHint.setFillColor(sf::Color(120, 120, 140));

    // 6个商店项目
    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        ShopItem item = static_cast<ShopItem>(i);
        ShopButton sb;
        float y = 180.0f + i * 70.0f;

        sb.bg.setSize(sf::Vector2f(600, 56));
        sb.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, y);
        sb.bg.setFillColor(sf::Color(30, 30, 50));
        sb.bg.setOutlineColor(sf::Color(70, 70, 100));
        sb.bg.setOutlineThickness(1);

        sb.nameText.setFont(m_menuFont);
        sb.nameText.setCharacterSize(20);
        sb.nameText.setFillColor(sf::Color::White);

        sb.levelText.setFont(m_menuFont);
        sb.levelText.setCharacterSize(16);
        sb.levelText.setFillColor(sf::Color(180, 180, 200));

        sb.costText.setFont(m_menuFont);
        sb.costText.setCharacterSize(16);
        sb.costText.setFillColor(sf::Color(255, 215, 0));

        sb.buyBtn.setSize(sf::Vector2f(100, 34));
        sb.buyBtn.setFillColor(sf::Color(40, 100, 40));
        sb.buyBtn.setOutlineColor(sf::Color(100, 200, 100));
        sb.buyBtn.setOutlineThickness(1);

        sb.buyLabel.setFont(m_menuFont);
        sb.buyLabel.setCharacterSize(16);
        sb.buyLabel.setFillColor(sf::Color::White);
        sb.buyLabel.setString(LangManager::get(TextKey::Shop_Buy));

        m_shopButtons.push_back(sb);
    }

    refreshShopTexts();
}

void Game::refreshShopTexts()
{
    m_shopGoldText.setString(
        std::wstring(LangManager::get(TextKey::Shop_Gold)) + L": " +
        std::to_wstring(m_playerData.totalGold));

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        auto &sb = m_shopButtons[i];
        ShopItem item = static_cast<ShopItem>(i);
        int lv = m_playerData.shopLevels[i];
        int cost = m_playerData.getUpgradeCost(item);

        // 获取描述键
        TextKey descKeys[] = {TextKey::Shop_Desc_StartGold, TextKey::Shop_Desc_Discount,
                              TextKey::Shop_Desc_Damage, TextKey::Shop_Desc_Range,
                              TextKey::Shop_Desc_Lives, TextKey::Shop_Desc_FireRate};

        // 名称键
        TextKey nameKeys[] = {TextKey::Shop_StartGold, TextKey::Shop_Discount,
                              TextKey::Shop_Damage, TextKey::Shop_Range,
                              TextKey::Shop_Lives, TextKey::Shop_FireRate};

        sb.nameText.setString(LangManager::get(nameKeys[i]));
        sb.levelText.setString(std::wstring(LangManager::get(descKeys[i])) +
                               L"  [Lv" + std::to_wstring(lv) + L"/" + std::to_wstring(SHOP_MAX_LEVEL) + L"]");

        if (lv >= SHOP_MAX_LEVEL)
        {
            sb.costText.setString(LangManager::get(TextKey::Shop_Max));
            sb.buyBtn.setFillColor(sf::Color(60, 60, 60));
            sb.buyLabel.setString(LangManager::get(TextKey::Shop_Max));
        }
        else
        {
            sb.costText.setString(std::to_wstring(cost) + L" " +
                                  std::wstring(LangManager::get(TextKey::CharSelect_Gold)));
            bool canBuy = m_playerData.totalGold >= cost;
            sb.buyBtn.setFillColor(canBuy ? sf::Color(40, 100, 40) : sf::Color(60, 40, 40));
            sb.buyLabel.setString(LangManager::get(TextKey::Shop_Buy));
        }
    }
}

void Game::renderShop()
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30));
    m_window.draw(bg);

    sf::FloatRect tb = m_shopTitleText.getLocalBounds();
    m_shopTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_shopTitleText.setPosition(WINDOW_WIDTH / 2.0f, 80);
    m_window.draw(m_shopTitleText);

    sf::FloatRect gb = m_shopGoldText.getLocalBounds();
    m_shopGoldText.setOrigin(gb.width / 2, gb.height / 2);
    m_shopGoldText.setPosition(WINDOW_WIDTH / 2.0f, 130);
    m_window.draw(m_shopGoldText);

    for (auto &sb : m_shopButtons)
    {
        m_window.draw(sb.bg);

        sb.nameText.setPosition(sb.bg.getPosition().x + 10, sb.bg.getPosition().y + 4);
        m_window.draw(sb.nameText);

        sb.levelText.setPosition(sb.bg.getPosition().x + 10, sb.bg.getPosition().y + 28);
        m_window.draw(sb.levelText);

        sb.costText.setPosition(sb.bg.getPosition().x + 400, sb.bg.getPosition().y + 16);
        m_window.draw(sb.costText);

        sb.buyBtn.setPosition(sb.bg.getPosition().x + 490, sb.bg.getPosition().y + 11);
        m_window.draw(sb.buyBtn);

        sf::FloatRect bl = sb.buyLabel.getLocalBounds();
        sb.buyLabel.setOrigin(bl.width / 2, bl.height / 2);
        sb.buyLabel.setPosition(sb.buyBtn.getPosition().x + 50, sb.buyBtn.getPosition().y + 17);
        m_window.draw(sb.buyLabel);
    }

    sf::FloatRect hb = m_shopBackHint.getLocalBounds();
    m_shopBackHint.setOrigin(hb.width / 2, hb.height / 2);
    m_shopBackHint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(m_shopBackHint);
}

void Game::processShopEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;
        for (auto &sb : m_shopButtons)
        {
            bool inside = sb.buyBtn.getGlobalBounds().contains(mx, my);
            sb.hovered = inside;
        }
        return;
    }
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        auto &sb = m_shopButtons[i];
        if (sb.buyBtn.getGlobalBounds().contains(mx, my))
        {
            ShopItem item = static_cast<ShopItem>(i);
            if (m_playerData.canUpgrade(item) &&
                m_playerData.totalGold >= m_playerData.getUpgradeCost(item))
            {
                m_playerData.upgrade(item);
                m_playerData.save(PlayerData::makeSavePath(m_playerData.name));
                refreshShopTexts();
            }
            break;
        }
    }
}

// ============================================================
//  确认对话框
// ============================================================

void Game::buildConfirmUI()
{
    m_confirmTitleText.setFont(m_menuFont);
    m_confirmTitleText.setString(LangManager::get(TextKey::NewGame_WarnTitle));
    m_confirmTitleText.setCharacterSize(36);
    m_confirmTitleText.setFillColor(sf::Color(255, 80, 80));
    m_confirmTitleText.setStyle(sf::Text::Bold);

    m_confirmMsgText.setFont(m_menuFont);
    m_confirmMsgText.setString(LangManager::get(TextKey::NewGame_WarnMsg));
    m_confirmMsgText.setCharacterSize(20);
    m_confirmMsgText.setFillColor(sf::Color(200, 200, 200));

    m_confirmYesBtn.setSize(sf::Vector2f(200, 44));
    m_confirmYesBtn.setFillColor(sf::Color(150, 40, 40));
    m_confirmYesBtn.setOutlineColor(sf::Color(255, 100, 100));
    m_confirmYesBtn.setOutlineThickness(2);
    m_confirmYesLabel.setFont(m_menuFont);
    m_confirmYesLabel.setString(LangManager::get(TextKey::NewGame_ConfirmYes));
    m_confirmYesLabel.setCharacterSize(18);
    m_confirmYesLabel.setFillColor(sf::Color::White);

    m_confirmNoBtn.setSize(sf::Vector2f(200, 44));
    m_confirmNoBtn.setFillColor(sf::Color(50, 50, 70));
    m_confirmNoBtn.setOutlineColor(sf::Color(100, 100, 140));
    m_confirmNoBtn.setOutlineThickness(2);
    m_confirmNoLabel.setFont(m_menuFont);
    m_confirmNoLabel.setString(LangManager::get(TextKey::NewGame_ConfirmNo));
    m_confirmNoLabel.setCharacterSize(18);
    m_confirmNoLabel.setFillColor(sf::Color::White);
}

void Game::renderConfirmDialog()
{
    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // 对话框背景
    sf::RectangleShape dlgBg(sf::Vector2f(440, 240));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - 220, WINDOW_HEIGHT / 2.0f - 120);
    dlgBg.setFillColor(sf::Color(25, 25, 45));
    dlgBg.setOutlineColor(sf::Color(255, 80, 80));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    sf::FloatRect tb = m_confirmTitleText.getLocalBounds();
    m_confirmTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_confirmTitleText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 80);
    m_window.draw(m_confirmTitleText);

    sf::FloatRect mb = m_confirmMsgText.getLocalBounds();
    m_confirmMsgText.setOrigin(mb.width / 2, mb.height / 2);
    m_confirmMsgText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 20);
    m_window.draw(m_confirmMsgText);

    m_confirmYesBtn.setPosition(WINDOW_WIDTH / 2.0f - 220, WINDOW_HEIGHT / 2.0f + 40);
    m_window.draw(m_confirmYesBtn);
    sf::FloatRect yb = m_confirmYesLabel.getLocalBounds();
    m_confirmYesLabel.setOrigin(yb.width / 2, yb.height / 2);
    m_confirmYesLabel.setPosition(WINDOW_WIDTH / 2.0f - 120, WINDOW_HEIGHT / 2.0f + 62);
    m_window.draw(m_confirmYesLabel);

    m_confirmNoBtn.setPosition(WINDOW_WIDTH / 2.0f + 20, WINDOW_HEIGHT / 2.0f + 40);
    m_window.draw(m_confirmNoBtn);
    sf::FloatRect nb = m_confirmNoLabel.getLocalBounds();
    m_confirmNoLabel.setOrigin(nb.width / 2, nb.height / 2);
    m_confirmNoLabel.setPosition(WINDOW_WIDTH / 2.0f + 120, WINDOW_HEIGHT / 2.0f + 62);
    m_window.draw(m_confirmNoLabel);
}

// ============================================================
//  战役 & 自定义 委托
// ============================================================

void Game::processCampaignEvents(const sf::Event &event)
{
    LevelConfig chosen;
    if (m_campaignScreen.update(event, m_window, chosen, m_playerData.unlockedLevels))
        newGame(chosen);
}

void Game::renderCampaign()
{
    m_campaignScreen.draw(m_window, m_playerData.unlockedLevels);
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
