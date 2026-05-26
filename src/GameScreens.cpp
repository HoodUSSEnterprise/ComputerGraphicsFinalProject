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
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 150);

    m_subtitleText.setFont(m_menuFont);
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_subtitleText.setCharacterSize(18);
    m_subtitleText.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect sb = m_subtitleText.getLocalBounds();
    m_subtitleText.setOrigin(sb.width / 2, sb.height / 2);
    m_subtitleText.setPosition(WINDOW_WIDTH / 2.0f, 220);

    TextKey btnKeys[] = {TextKey::NewGame, TextKey::LoadGame, TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    for (int i = 0; i < 5; ++i)
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(280, 56));
        btn.bg.setOrigin(140, 28);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 300 + i * 72.0f);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(btnKeys[i]));
        btn.label.setCharacterSize(24);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 300 + i * 72.0f - 4);
        btn.hovered = false;
        m_menuButtons.push_back(btn);
    }
}

void Game::loadMenuFont()
{
    std::vector<std::string> paths = {LangManager::getFontPath()};
    std::string lang = LangManager::currentLangName();
    if (lang == "zh") paths.push_back("fonts/simhei.ttf");
    paths.push_back("fonts/arial.ttf");

    bool loaded = false;
    for (const auto &p : paths)
        if (m_menuFont.loadFromFile(p)) { std::cout << "[Font] menu loaded: " << p << std::endl; loaded = true; break; }
    if (!loaded) std::cerr << "[Font] WARNING: menu font failed to load!" << std::endl;

    m_titleText.setFont(m_menuFont);
    m_subtitleText.setFont(m_menuFont);
    for (auto &btn : m_menuButtons) btn.label.setFont(m_menuFont);
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
        if (m_menuButtons[i].bg.getGlobalBounds().contains(mx, my)) return static_cast<int>(i);
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
    m_window.draw(m_titleText);
    m_window.draw(m_subtitleText);
    for (const auto &btn : m_menuButtons) { m_window.draw(btn.bg); m_window.draw(btn.label); }

    sf::Text langText;
    langText.setFont(m_menuFont);
    langText.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    langText.setCharacterSize(16); langText.setFillColor(sf::Color(100, 200, 100));
    sf::FloatRect lb = langText.getLocalBounds();
    langText.setOrigin(lb.width / 2, lb.height / 2);
    langText.setPosition(WINDOW_WIDTH / 2.0f, 600);
    m_window.draw(langText);

    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(LangManager::get(TextKey::MenuHint));
    hint.setCharacterSize(14); hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);
}

void Game::processMenuEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        updateMenuHover(worldPos.x, worldPos.y);
        return;
    }
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    int idx = getMenuButtonIndex(worldPos.x, worldPos.y);
    switch (idx)
    {
    case 0: m_state = GameState::CampaignSelect; break;
    case 1: if (!loadGame()) newGame(); break;
    case 2: m_state = GameState::CustomSetup; break;
    case 3: m_state = GameState::Settings; buildSettingsUI(); break;
    case 4: m_window.close(); break;
    default: break;
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
    m_volKnob.setRadius(12); m_volKnob.setOrigin(12, 12);
    m_volKnob.setFillColor(sf::Color(255, 215, 0));
    m_bgmLabel.setFont(m_menuFont); m_bgmLabel.setCharacterSize(28);
    m_bgmLabel.setFillColor(sf::Color::White);
}

void Game::applyVolume() { m_bgm.setVolume(m_volume); }

int Game::getSettingsButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_settingsButtons.size(); ++i)
        if (m_settingsButtons[i].bg.getGlobalBounds().contains(mx, my)) return static_cast<int>(i);
    return -1;
}

void Game::buildSettingsUI()
{
    m_settingsButtons.clear();
    auto makeArrow = [&](float x, float y, const std::string &label) {
        MenuButton btn; btn.bg.setSize(sf::Vector2f(50, 50)); btn.bg.setOrigin(25, 25);
        btn.bg.setPosition(x, y); btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140)); btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont); btn.label.setString(label); btn.label.setCharacterSize(30);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds(); btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(x, y - 4); btn.hovered = false;
        m_settingsButtons.push_back(btn);
    };
    makeArrow(WINDOW_WIDTH / 2.0f - 180, 300, "<");
    makeArrow(WINDOW_WIDTH / 2.0f + 180, 300, ">");

    m_langLabel.setFont(m_menuFont);
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langLabel.setCharacterSize(24); m_langLabel.setFillColor(sf::Color(180, 180, 200));
    m_langValue.setFont(m_menuFont);
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    m_langValue.setCharacterSize(22); m_langValue.setFillColor(sf::Color::Yellow);

    {
        MenuButton btn; btn.bg.setSize(sf::Vector2f(280, 56)); btn.bg.setOrigin(140, 28);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 400); btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140)); btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        btn.label.setCharacterSize(24); btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds(); btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 396); btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
    {
        MenuButton btn; btn.bg.setSize(sf::Vector2f(200, 50)); btn.bg.setOrigin(100, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, 520); btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140)); btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont); btn.label.setString(LangManager::get(TextKey::Back));
        btn.label.setCharacterSize(22); btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds(); btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 516); btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
}

void Game::renderSettings()
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30)); m_window.draw(bg);
    for (int i = 0; i < 10; ++i) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_WIDTH, 2));
        line.setFillColor(sf::Color(30, 30, 50)); line.setPosition(0, i * 80.0f); m_window.draw(line);
    }
    sf::Text title;
    title.setFont(m_menuFont); title.setString(LangManager::get(TextKey::Settings));
    title.setCharacterSize(50); title.setFillColor(sf::Color(255, 215, 0)); title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2); title.setPosition(WINDOW_WIDTH / 2.0f, 120);
    m_window.draw(title);
    m_langLabel.setPosition(WINDOW_WIDTH / 2.0f - 130, 290);
    m_langValue.setPosition(WINDOW_WIDTH / 2.0f - 30, 290);
    m_window.draw(m_langLabel); m_window.draw(m_langValue);

    float trackX = WINDOW_WIDTH / 2.0f - 150, trackY = 360;
    m_volTrack.setPosition(trackX, trackY); m_window.draw(m_volTrack);
    float knobX = trackX + (m_volume / 100.0f) * m_volTrack.getSize().x;
    m_volKnob.setPosition(knobX, trackY + m_volTrack.getSize().y / 2.0f);
    m_window.draw(m_volKnob);

    sf::Text volLabel;
    volLabel.setFont(m_menuFont);
    volLabel.setString(std::wstring(LangManager::get(TextKey::Volume)) + L": " + std::to_wstring(static_cast<int>(m_volume)) + L"%");
    volLabel.setCharacterSize(20); volLabel.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect vl = volLabel.getLocalBounds();
    volLabel.setOrigin(vl.width / 2, vl.height / 2); volLabel.setPosition(WINDOW_WIDTH / 2.0f, 330);
    m_window.draw(volLabel);
    for (const auto &btn : m_settingsButtons) { m_window.draw(btn.bg); m_window.draw(btn.label); }
    sf::Text hint; hint.setFont(m_menuFont); hint.setString("ESC: Back to Menu");
    hint.setCharacterSize(14); hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2); hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);
}

void Game::processSettingsEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (auto &btn : m_settingsButtons) {
            bool inside = btn.bg.getGlobalBounds().contains(worldPos.x, worldPos.y); btn.hovered = inside;
            btn.bg.setFillColor(inside ? sf::Color(70, 70, 100) : sf::Color(50, 50, 70));
            btn.bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
            btn.label.setFillColor(inside ? sf::Color(255, 215, 0) : sf::Color::White);
        }
        if (m_draggingVol) {
            float nx = std::max(m_volTrack.getPosition().x, std::min(worldPos.x, m_volTrack.getPosition().x + m_volTrack.getSize().x));
            m_volume = (nx - m_volTrack.getPosition().x) / m_volTrack.getSize().x * 100.0f;
            m_volKnob.setPosition(nx, m_volKnob.getPosition().y); applyVolume();
        }
        return;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        if (m_volKnob.getGlobalBounds().contains(worldPos.x, worldPos.y)) { m_draggingVol = true; return; }
        int idx = getSettingsButtonIndex(worldPos.x, worldPos.y);
        switch (idx) {
        case 0: case 1: {
            static const char *langs[] = {"assets/lang_en.json", "assets/lang_zh.json"};
            static int cur = 0;
            cur = (idx == 0) ? (cur - 1 + 2) % 2 : (cur + 1) % 2;
            LangManager::loadLanguage(langs[cur]);
            loadMenuFont(); m_ui.reloadFont(); refreshAllTexts();
            break;
        }
        case 2:
            m_bgmOn = !m_bgmOn;
            if (m_bgmOn) { if (m_bgm.getStatus() != sf::Music::Playing) m_bgm.play(); }
            else m_bgm.pause();
            m_bgmLabel.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
            break;
        case 3: m_state = GameState::Menu; break;
        default: break;
        }
    }
    if (event.type == sf::Event::MouseButtonReleased) m_draggingVol = false;
}

// ============================================================
//  文本刷新
// ============================================================

void Game::refreshAllTexts()
{
    TextKey btnKeys[] = {TextKey::NewGame, TextKey::LoadGame, TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    for (size_t i = 0; i < m_menuButtons.size() && i < 5; ++i)
        m_menuButtons[i].label.setString(LangManager::get(btnKeys[i]));
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    if (!m_settingsButtons.empty()) {
        m_settingsButtons[2].label.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        m_settingsButtons[3].label.setString(LangManager::get(TextKey::Back));
    }
    m_campaignScreen.refreshTexts(); m_campaignScreen.reloadFont();
    m_customScreen.refreshTexts(); m_customScreen.reloadFont();
}

// ============================================================
//  战役 & 自定义 委托
// ============================================================

void Game::processCampaignEvents(const sf::Event &event)
{
    LevelConfig chosen;
    if (m_campaignScreen.update(event, m_window, chosen)) newGame(chosen);
}

void Game::renderCampaign() { m_campaignScreen.draw(m_window); }

void Game::processCustomSetupEvents(const sf::Event &event)
{
    CustomParams params;
    int result = m_customScreen.update(event, m_window, params);
    if (result == 1) {
        LevelConfig cfg;
        cfg.mapFile = "assets/maps/grassland/1-1.txt"; cfg.biome = Biome::Grassland;
        cfg.startGold = params.startGold; cfg.startLives = params.startLives;
        cfg.waveCount = params.waves; cfg.baseEnemies = params.enemiesPerWave;
        cfg.speedMul = params.speedMul; cfg.hpMul = params.hpMul; cfg.id = "custom";
        newGame(cfg);
    } else if (result == 2) {
        m_state = GameState::Menu;
    }
}

void Game::renderCustomSetup() { m_customScreen.draw(m_window); }
