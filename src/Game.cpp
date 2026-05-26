#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

// ============================================================
//  构�?/ 主循�?
// ============================================================

Game::Game()
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + 100),
               "Tower Defense - SFML",
               sf::Style::Default),
      m_state(GameState::Menu), m_selectedTowerType(TowerType::Arrow), m_gold(200), m_lives(20),
      m_volume(80.0f), m_bgmOn(true), m_draggingVol(false)
{

    m_window.setFramerateLimit(60);

    m_view = sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    m_window.setView(m_view);

    LangManager::loadLanguage("assets/lang_en.json");
    m_ui.reloadFont();
    m_campaignScreen.refreshTexts();
    m_customScreen.refreshTexts();
    initMenu();
    initSettings();

    // 尝试加载背景音乐
    if (m_bgm.openFromFile("sound/bgm.mp3"))
    {
        m_bgm.setLoop(true);
        m_bgm.setVolume(m_volume);
        if (m_bgmOn)
            m_bgm.play();
    }
}

void Game::run()
{
    while (m_window.isOpen())
    {
        float dt = m_clock.restart().asSeconds();
        if (dt > 0.1f)
            dt = 0.1f;

        processEvents();

        if (m_state == GameState::Playing)
        {
            update(dt);
        }

        render();
    }
}

// ============================================================
//  状态切换
// ============================================================

void Game::newGame()
{
    // 默认关卡（兼容加载游戏）
    static LevelConfig defaultCfg = getCampaignLevels()[0];
    newGame(defaultCfg);
}

void Game::newGame(const LevelConfig &cfg)
{
    m_towers.clear();
    m_enemies.clear();
    m_projectiles.clear();
    m_map = Map();
    m_map.loadFromFile(cfg.mapFile.c_str());
    // 根据群系加载纹理
    static const char *biomeNames[] = {"grassland", "desert", "hell", "community"};
    int bIdx = static_cast<int>(cfg.biome);
    if (bIdx >= 0 && bIdx < 4)
    {
        m_map.loadBiomeTextures(biomeNames[bIdx]);
    }
    m_map.loadEndTextures();
    m_gold = cfg.startGold;
    m_lives = cfg.startLives;
    m_waveManager = WaveManager();
    m_waveManager.setCustomWaves(cfg.waveCount, cfg.baseEnemies, cfg.speedMul, cfg.hpMul);
    m_selectedTowerType = TowerType::Arrow;
    m_state = GameState::Playing;
}

void Game::saveGame()
{
    std::ofstream file("save.dat");
    if (!file)
        return;

    file << m_gold << '\n';
    file << m_lives << '\n';
    file << static_cast<int>(m_selectedTowerType) << '\n';
    file << m_waveManager.getCurrentWave() << '\n';
    file << m_towers.size() << '\n';

    for (const auto &t : m_towers)
    {
        file << static_cast<int>(t->getType()) << ' '
             << t->getPosition().x << ' ' << t->getPosition().y << '\n';
    }
}

bool Game::loadGame()
{
    std::ifstream file("save.dat");
    if (!file)
        return false;

    int gold, lives, selType, wave;
    file >> gold >> lives >> selType >> wave;

    m_gold = gold;
    m_lives = lives;
    m_selectedTowerType = static_cast<TowerType>(selType);

    // 重建 wave manager 到指定波�?
    m_waveManager = WaveManager();
    for (int i = 0; i < wave; ++i)
    {
        m_waveManager.startNextWave();
    }

    // 读取�?
    size_t towerCount;
    file >> towerCount;
    m_towers.clear();
    m_map = Map();

    for (size_t i = 0; i < towerCount; ++i)
    {
        int type;
        float x, y;
        file >> type >> x >> y;
        m_towers.push_back(std::make_shared<Tower>(
            static_cast<TowerType>(type), sf::Vector2f(x, y)));
        auto grid = m_map.worldToGrid(x, y);
        m_map.setTile(grid.x, grid.y, TileType::Blocked);
    }

    m_enemies.clear();
    m_projectiles.clear();
    m_state = GameState::Playing;
    return true;
}

void Game::returnToMenu()
{
    m_state = GameState::Menu;
}

// ============================================================
//  事件处理
// ============================================================

void Game::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_window.close();
            return;
        }

        if (event.type == sf::Event::Resized)
        {
            handleResize();
            continue;
        }

        switch (m_state)
        {
        case GameState::Menu:
            processMenuEvents(event);
            break;
        case GameState::Settings:
            processSettingsEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                m_state = GameState::Menu;
            break;
        case GameState::CampaignSelect:
            processCampaignEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                m_state = GameState::Menu;
            break;
        case GameState::CustomSetup:
            processCustomSetupEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                m_state = GameState::Menu;
            break;
        case GameState::Playing:
            processPlayingEvents(event);
            break;
        case GameState::GameOver:
        case GameState::GameWon:
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::R)
                    newGame();
                else if (event.key.code == sf::Keyboard::Escape)
                    returnToMenu();
            }
            break;
        default:
            break;
        }
    }
}

void Game::handleResize()
{
    float winW = static_cast<float>(m_window.getSize().x);
    float winH = static_cast<float>(m_window.getSize().y);
    float viewW = WINDOW_WIDTH;
    float viewH = WINDOW_HEIGHT + 100;

    float windowRatio = winW / winH;
    float viewRatio = viewW / viewH;

    float vpW, vpH;
    if (windowRatio > viewRatio)
    {
        vpH = viewH;
        vpW = vpH * windowRatio;
    }
    else
    {
        vpW = viewW;
        vpH = vpW / windowRatio;
    }

    m_view.setSize(vpW, vpH);
    m_view.setCenter(viewW / 2.0f, viewH / 2.0f);
}

void Game::processMenuEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        updateMenuHover(worldPos.x, worldPos.y);
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed)
        return;
    if (event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    int idx = getMenuButtonIndex(worldPos.x, worldPos.y);
    switch (idx)
    {
    case 0:
        m_state = GameState::CampaignSelect;
        break;
    case 1:
        if (!loadGame())
            newGame();
        break;
    case 2:
        m_state = GameState::CustomSetup;
        break;
    case 3:
        m_state = GameState::Settings;
        buildSettingsUI();
        break;
    case 4:
        m_window.close();
        break;
    default:
        break;
    }
}

void Game::processSettingsEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));

        // 悬停设置按钮
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
            float nx = std::max(m_volTrack.getPosition().x,
                                std::min(worldPos.x,
                                         m_volTrack.getPosition().x + m_volTrack.getSize().x));
            float ratio = (nx - m_volTrack.getPosition().x) / m_volTrack.getSize().x;
            m_volume = ratio * 100.0f;
            m_volKnob.setPosition(nx, m_volKnob.getPosition().y);
            applyVolume();
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        // 检查音量滑�?
        if (m_volKnob.getGlobalBounds().contains(worldPos.x, worldPos.y))
        {
            m_draggingVol = true;
            return;
        }

        int idx = getSettingsButtonIndex(worldPos.x, worldPos.y);
        switch (idx)
        {
        case 0: // 语言向左
        {
            static const char *langs[] = {
                "assets/lang_en.json", "assets/lang_zh.json"};
            static int cur = 0;
            cur = (cur - 1 + 2) % 2;
            LangManager::loadLanguage(langs[cur]);
            loadMenuFont();
            m_ui.reloadFont();
            refreshAllTexts();
            break;
        }
        case 1: // 语言向右
        {
            static const char *langs[] = {
                "assets/lang_en.json", "assets/lang_zh.json"};
            static int cur = 0;
            cur = (cur + 1) % 2;
            LangManager::loadLanguage(langs[cur]);
            loadMenuFont();
            m_ui.reloadFont();
            refreshAllTexts();
            break;
        }
        case 2: // BGM 开�?
            m_bgmOn = !m_bgmOn;
            if (m_bgmOn)
            {
                if (m_bgm.getStatus() != sf::Music::Playing)
                    m_bgm.play();
            }
            else
            {
                m_bgm.pause();
            }
            // 刷新 BGM 标签
            m_bgmLabel.setString(std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
            break;
        case 3: // Back
            m_state = GameState::Menu;
            break;
        default:
            break;
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonReleased)
    {
        m_draggingVol = false;
    }
}

void Game::processPlayingEvents(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        switch (event.key.code)
        {
        case sf::Keyboard::Num1:
            m_selectedTowerType = TowerType::Arrow;
            break;
        case sf::Keyboard::Num2:
            m_selectedTowerType = TowerType::Cannon;
            break;
        case sf::Keyboard::Num3:
            m_selectedTowerType = TowerType::Ice;
            break;
        case sf::Keyboard::Space:
            if (m_waveManager.canStartWave())
                m_waveManager.startNextWave();
            break;
        case sf::Keyboard::Escape:
            saveGame();
            returnToMenu();
            break;
        default:
            break;
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x;
    float my = worldPos.y;

    if (event.mouseButton.button == sf::Mouse::Left)
    {
        // 点击底部UI面板
        if (m_ui.isClickOnUI(mx, my))
        {
            if (m_ui.isStartWaveClicked(mx, my))
            {
                if (m_waveManager.canStartWave())
                {
                    m_waveManager.startNextWave();
                    int waveIdx = m_waveManager.getCurrentWave();
                    int total = m_waveManager.getTotalWaves();
                    if (waveIdx == total / 2 && total > 2)
                        m_map.switchPhase();
                    if (waveIdx > 1)
                        m_gold += 50 + (waveIdx - 1) * 10;
                    m_ui.showMessage(std::wstring(LangManager::get(TextKey::Wave)) + L" " + std::to_wstring(waveIdx) + L" " + LangManager::get(TextKey::Msg_WaveStarted));
                }
            }
            hidePopup();
            return;
        }

        // 弹出菜单已打开 → 处理菜单点击
        if (m_popupType == PopupType::Build)
        {
            auto grid = m_map.worldToGrid(mx, my);
            float px = m_popupPos.x, py = m_popupPos.y;
            // 三个建造按钮垂直排列
            for (int i = 0; i < 3; ++i)
            {
                if (mx >= px && mx <= px + 140 && my >= py + i * 38 && my <= py + i * 38 + 34)
                {
                    TowerType tt = static_cast<TowerType>(i);
                    int cost = Tower::getStats(tt).cost;
                    if (m_gold >= cost && m_map.canPlaceTower(grid.x, grid.y))
                    {
                        sf::Vector2f center = m_map.gridToWorld(grid.x, grid.y);
                        m_towers.push_back(std::make_shared<Tower>(tt, center));
                        m_gold -= cost;
                        m_map.setTile(grid.x, grid.y, TileType::Blocked);
                    }
                    else
                    {
                        m_ui.showMessage(LangManager::get(TextKey::Msg_NoGold));
                    }
                    hidePopup();
                    return;
                }
            }
            hidePopup();
            return;
        }

        if (m_popupType == PopupType::Tower)
        {
            float px = m_popupPos.x, py = m_popupPos.y;
            auto twr = m_popupTower.lock();
            if (twr)
            {
                // 升级按钮
                if (mx >= px && mx <= px + 140 && my >= py && my <= py + 34)
                {
                    if (twr->canUpgrade() && m_gold >= twr->getUpgradeCost())
                    {
                        m_gold -= twr->getUpgradeCost();
                        twr->upgrade();
                    }
                    hidePopup();
                    return;
                }
                // 出售按钮
                if (mx >= px && mx <= px + 140 && my >= py + 38 && my <= py + 72)
                {
                    int refund = twr->getSellValue();
                    m_gold += refund;
                    auto grid = m_map.worldToGrid(twr->getPosition().x, twr->getPosition().y);
                    m_map.setTile(grid.x, grid.y, TileType::Grass);
                    m_towers.erase(std::remove_if(m_towers.begin(), m_towers.end(),
                        [&](auto &t) { return t.get() == twr.get(); }), m_towers.end());
                    m_ui.showMessage(LangManager::get(TextKey::Msg_Sold) + std::to_wstring(refund));
                    hidePopup();
                    return;
                }
            }
            hidePopup();
            return;
        }

        // 没有弹出菜单 → 点击地图
        auto grid = m_map.worldToGrid(mx, my);
        if (m_map.getTile(grid.x, grid.y) == TileType::Grass)
        {
            showBuildPopup(mx, my);
        }
        else
        {
            // 检查是否点了已有的塔
            for (auto &t : m_towers)
            {
                float dx = t->getPosition().x - mx;
                float dy = t->getPosition().y - my;
                if (std::sqrt(dx * dx + dy * dy) < TILE_SIZE / 2.0f)
                {
                    showTowerPopup(mx, my);
                    m_popupTower = t;
                    return;
                }
            }
            hidePopup();
        }
    }
    else if (event.mouseButton.button == sf::Mouse::Right)
    {
        hidePopup();
    }
}

// ============================================================
//  更新
// ============================================================

void Game::update(float dt)
{
    updateEnemies(dt);
    updateTowers(dt);
    towerFindTargets();
    updateProjectiles(dt);
    checkProjectileCollisions();

    m_waveManager.update(dt, m_enemies, m_map.getWaypoints());

    if (m_waveManager.areAllWavesComplete() && m_enemies.empty())
    {
        m_state = GameState::GameWon;
    }

    m_ui.update(dt, m_gold, m_lives,
                m_waveManager.getCurrentWave(),
                m_waveManager.getTotalWaves(),
                m_selectedTowerType);

    if (m_lives <= 0)
    {
        m_state = GameState::GameOver;
    }
}

// ============================================================
//  渲染
// ============================================================

void Game::render()
{
    m_window.clear(sf::Color(20, 20, 30));
    m_window.setView(m_view);

    switch (m_state)
    {
    case GameState::Menu:
        renderMenu();
        break;
    case GameState::Settings:
        renderSettings();
        break;
    case GameState::CampaignSelect:
        renderCampaign();
        break;
    case GameState::CustomSetup:
        renderCustomSetup();
        break;
    case GameState::Playing:
        renderPlaying();
        break;
    case GameState::GameOver:
    case GameState::GameWon:
        renderPlaying();
        renderEndScreen();
        break;
    default:
        break;
    }

    m_window.display();
}

// ============================================================
//  菜单画面
// ============================================================

void Game::initMenu()
{
    loadMenuFont();

    // 标题
    m_titleText.setFont(m_menuFont);
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_titleText.setCharacterSize(60);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 150);

    // 副标题
    m_subtitleText.setFont(m_menuFont);
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_subtitleText.setCharacterSize(18);
    m_subtitleText.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect sb = m_subtitleText.getLocalBounds();
    m_subtitleText.setOrigin(sb.width / 2, sb.height / 2);
    m_subtitleText.setPosition(WINDOW_WIDTH / 2.0f, 220);

    // 按钮: New Game, Load Game, Custom Mode, Settings, Exit
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
    // 按优先级尝试字体路径：JSON指定 �?系统回退
    std::vector<std::string> paths = {LangManager::getFontPath()};

    // 中文回退：Windows 系统字体
    std::string lang = LangManager::currentLangName();
    if (lang == "zh")
    {
        paths.push_back("fonts/simhei.ttf");
    }
    // 通用回退
    paths.push_back("fonts/arial.ttf");

    bool loaded = false;
    for (const auto &p : paths)
    {
        if (m_menuFont.loadFromFile(p))
        {
            std::cout << "[Font] menu loaded: " << p << std::endl;
            loaded = true;
            break;
        }
    }
    if (!loaded)
        std::cerr << "[Font] WARNING: menu font failed to load!" << std::endl;

    m_titleText.setFont(m_menuFont);
    m_subtitleText.setFont(m_menuFont);
    for (auto &btn : m_menuButtons)
        btn.label.setFont(m_menuFont);
}

void Game::updateMenuHover(float mx, float my)
{
    for (auto &btn : m_menuButtons)
    {
        bool inside = btn.bg.getGlobalBounds().contains(mx, my);
        btn.hovered = inside;
        if (inside)
        {
            btn.bg.setFillColor(sf::Color(70, 70, 100));
            btn.bg.setOutlineColor(sf::Color(255, 215, 0));
            btn.label.setFillColor(sf::Color(255, 215, 0));
        }
        else
        {
            btn.bg.setFillColor(sf::Color(50, 50, 70));
            btn.bg.setOutlineColor(sf::Color(100, 100, 140));
            btn.label.setFillColor(sf::Color::White);
        }
    }
}

int Game::getMenuButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_menuButtons.size(); ++i)
    {
        if (m_menuButtons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    }
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

    for (const auto &btn : m_menuButtons)
    {
        m_window.draw(btn.bg);
        m_window.draw(btn.label);
    }

    // 当前语言显示
    sf::Text langText;
    langText.setFont(m_menuFont);
    langText.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    langText.setCharacterSize(16);
    langText.setFillColor(sf::Color(100, 200, 100));
    sf::FloatRect lb = langText.getLocalBounds();
    langText.setOrigin(lb.width / 2, lb.height / 2);
    langText.setPosition(WINDOW_WIDTH / 2.0f, 600);
    m_window.draw(langText);

    // 底部提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(LangManager::get(TextKey::MenuHint));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(hint);
}

// ============================================================
//  设置画面
// ============================================================

void Game::initSettings()
{
    // 音量滑轨
    m_volTrack.setSize(sf::Vector2f(300, 8));
    m_volTrack.setFillColor(sf::Color(60, 60, 80));
    m_volTrack.setOutlineColor(sf::Color(100, 100, 140));
    m_volTrack.setOutlineThickness(1);

    // 音量旋钮
    m_volKnob.setRadius(12);
    m_volKnob.setOrigin(12, 12);
    m_volKnob.setFillColor(sf::Color(255, 215, 0));

    // BGM 标签
    m_bgmLabel.setFont(m_menuFont);
    m_bgmLabel.setCharacterSize(28);
    m_bgmLabel.setFillColor(sf::Color::White);
}

void Game::buildSettingsUI()
{
    m_settingsButtons.clear();

    // ---- 语言选择 (�?右箭�? ----
    // 左箭�?
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(50, 50));
        btn.bg.setOrigin(25, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 180, 300);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString("<");
        btn.label.setCharacterSize(30);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f - 180, 296);
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }
    // 右箭�?
    {
        MenuButton btn;
        btn.bg.setSize(sf::Vector2f(50, 50));
        btn.bg.setOrigin(25, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f + 180, 300);
        btn.bg.setFillColor(sf::Color(50, 50, 70));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(">");
        btn.label.setCharacterSize(30);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f + 180, 296);
        btn.hovered = false;
        m_settingsButtons.push_back(btn);
    }

    m_langLabel.setFont(m_menuFont);
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langLabel.setCharacterSize(24);
    m_langLabel.setFillColor(sf::Color(180, 180, 200));

    m_langValue.setFont(m_menuFont);
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    m_langValue.setCharacterSize(22);
    m_langValue.setFillColor(sf::Color::Yellow);

    // ---- BGM 开关按�?----
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

    // ---- 返回按钮 ----
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

    // 标题
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

    // 语言选择�?
    m_langLabel.setPosition(WINDOW_WIDTH / 2.0f - 130, 290);
    m_langValue.setPosition(WINDOW_WIDTH / 2.0f - 30, 290);
    m_window.draw(m_langLabel);
    m_window.draw(m_langValue);

    // 音量滑块
    float trackX = WINDOW_WIDTH / 2.0f - 150;
    float trackY = 360;
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

    // 设置按钮
    for (const auto &btn : m_settingsButtons)
    {
        m_window.draw(btn.bg);
        m_window.draw(btn.label);
    }

    // 底部提示
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

void Game::applyVolume()
{
    m_bgm.setVolume(m_volume);
}

void Game::refreshAllTexts()
{
    TextKey btnKeys[] = {TextKey::NewGame, TextKey::LoadGame, TextKey::CustomMode, TextKey::Settings, TextKey::Exit};
    for (size_t i = 0; i < m_menuButtons.size() && i < 5; ++i)
        m_menuButtons[i].label.setString(LangManager::get(btnKeys[i]));
    m_titleText.setString(LangManager::get(TextKey::Title));
    m_subtitleText.setString(LangManager::get(TextKey::Subtitle));
    m_langLabel.setString(LangManager::get(TextKey::Language_Label));
    m_langValue.setString(L"[" + std::wstring(LangManager::currentLangName().begin(), LangManager::currentLangName().end()) + L"]");
    // 更新设置页按钮
    if (!m_settingsButtons.empty())
    {
        m_settingsButtons[2].label.setString(
            std::wstring(L"BGM: ") + (m_bgmOn ? LangManager::get(TextKey::BGM_On) : LangManager::get(TextKey::BGM_Off)));
        m_settingsButtons[3].label.setString(LangManager::get(TextKey::Back));
    }
    // 更新战役和自定义界面
    m_campaignScreen.refreshTexts();
    m_campaignScreen.reloadFont();
    m_customScreen.refreshTexts();
    m_customScreen.reloadFont();
}

int Game::getSettingsButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_settingsButtons.size(); ++i)
    {
        if (m_settingsButtons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    }
    return -1;
}

// ============================================================
//  游戏画面
// ============================================================

void Game::renderPlaying()
{
    m_map.draw(m_window);

    for (const auto &tower : m_towers)
        tower->draw(m_window);
    for (const auto &proj : m_projectiles)
        proj->draw(m_window);
    for (const auto &enemy : m_enemies)
        enemy->draw(m_window);

    m_ui.draw(m_window);
    drawPopup();
}

// ============================================================
//  结束画面覆盖�?
// ============================================================

void Game::renderEndScreen()
{
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    sf::Text text;
    text.setFont(m_menuFont);
    text.setCharacterSize(48);
    text.setStyle(sf::Text::Bold);

    if (m_state == GameState::GameWon)
    {
        text.setString(LangManager::get(TextKey::Victory));
        text.setFillColor(sf::Color::Yellow);
    }
    else
    {
        text.setString(LangManager::get(TextKey::GameOver));
        text.setFillColor(sf::Color::Red);
    }

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.width / 2, bounds.height / 2);
    text.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 60);
    m_window.draw(text);

    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setCharacterSize(22);
    hint.setFillColor(sf::Color::White);
    hint.setString(std::wstring(LangManager::get(TextKey::PressR)) + L"    |    " + std::wstring(LangManager::get(TextKey::EscToMenu)));
    bounds = hint.getLocalBounds();
    hint.setOrigin(bounds.width / 2, bounds.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 20);
    m_window.draw(hint);
}

// ============================================================
//  游戏逻辑 (不变)
// ============================================================

void Game::handleTowerPlacement(float x, float y)
{
    auto grid = m_map.worldToGrid(x, y);
    if (!m_map.canPlaceTower(grid.x, grid.y))
    {
        m_ui.showMessage(LangManager::get(TextKey::Msg_CannotPlace));
        return;
    }

    int cost = Tower::getStats(m_selectedTowerType).cost;
    if (m_gold < cost)
    {
        m_ui.showMessage(std::wstring(LangManager::get(TextKey::Msg_NoGold)) + L" $" + std::to_wstring(cost));
        return;
    }

    sf::Vector2f center = m_map.gridToWorld(grid.x, grid.y);
    for (const auto &t : m_towers)
    {
        float dx = t->getPosition().x - center.x;
        float dy = t->getPosition().y - center.y;
        if (std::sqrt(dx * dx + dy * dy) < TILE_SIZE / 2.0f)
        {
            m_ui.showMessage(LangManager::get(TextKey::Msg_AlreadyHere));
            return;
        }
    }

    m_towers.push_back(std::make_shared<Tower>(m_selectedTowerType, center));
    m_gold -= cost;
    m_map.setTile(grid.x, grid.y, TileType::Blocked);

    TextKey towerKeys[] = {TextKey::Tower_Arrow, TextKey::Tower_Cannon, TextKey::Tower_Ice};
    m_ui.showMessage(std::wstring(LangManager::get(towerKeys[static_cast<int>(m_selectedTowerType)])) + L" " + LangManager::get(TextKey::Msg_Placed));
}

void Game::handleTowerSelection(float x, float y)
{
    m_selectedTowerType = m_ui.handleClick(x, y);
}

void Game::handleSellTower(float x, float y)
{
    for (auto it = m_towers.begin(); it != m_towers.end(); ++it)
    {
        float dx = (*it)->getPosition().x - x;
        float dy = (*it)->getPosition().y - y;
        if (std::sqrt(dx * dx + dy * dy) < TILE_SIZE / 2.0f)
        {
            int refund = (*it)->getCost() / 2;
            m_gold += refund;
            auto grid = m_map.worldToGrid((*it)->getPosition().x,
                                          (*it)->getPosition().y);
            m_map.setTile(grid.x, grid.y, TileType::Grass);
            m_towers.erase(it);
            m_ui.showMessage(std::wstring(LangManager::get(TextKey::Msg_Sold)) + std::to_wstring(refund));
            return;
        }
    }
}

void Game::updateTowers(float dt)
{
    for (auto &tower : m_towers)
        tower->update(dt);
}

void Game::updateProjectiles(float dt)
{
    for (auto &proj : m_projectiles)
        proj->update(dt);
}

void Game::updateEnemies(float dt)
{
    const auto &waypoints = m_map.getWaypoints();
    for (auto &enemy : m_enemies)
    {
        enemy->update(dt, waypoints);
    }

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
                       [this](const std::shared_ptr<Enemy> &e)
                       {
                           if (e->hasReachedEnd())
                           {
                               m_lives--;
                               return true;
                           }
                           return e->isDead();
                       }),
        m_enemies.end());
}

void Game::towerFindTargets()
{
    for (auto &tower : m_towers)
    {
        if (!tower->canFire())
            continue;

        std::shared_ptr<Enemy> bestTarget = nullptr;
        float bestDist = tower->getRange();

        for (auto &enemy : m_enemies)
        {
            if (enemy->isDead() || enemy->hasReachedEnd())
                continue;
            float dx = enemy->getPosition().x - tower->getPosition().x;
            float dy = enemy->getPosition().y - tower->getPosition().y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= tower->getRange() && (!bestTarget || dist < bestDist))
            {
                bestTarget = enemy;
                bestDist = dist;
            }
        }

        if (bestTarget)
        {
            m_projectiles.push_back(std::make_shared<Projectile>(
                tower->getPosition(), bestTarget,
                tower->getDamage(), 300.0f, tower->getType()));
            tower->resetFireTimer();
        }
    }
}

void Game::checkProjectileCollisions()
{
    for (auto &proj : m_projectiles)
    {
        if (proj->hasHit())
        {
            auto target = proj->getTarget();
            if (target && !target->isDead())
            {
                target->takeDamage(proj->getDamage());
                if (proj->getTowerType() == TowerType::Ice)
                {
                    target->applySlow(0.4f, 2.0f);
                }
                if (target->isDead())
                {
                    m_gold += target->getReward();
                }
            }
        }
    }

    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                       [](const std::shared_ptr<Projectile> &p)
                       {
                           return p->hasHit();
                       }),
        m_projectiles.end());
}

// ============================================================
//  战役选关画面
// ============================================================

void Game::processCampaignEvents(const sf::Event &event)
{
    LevelConfig chosen;
    if (m_campaignScreen.update(event, m_window, chosen))
    {
        newGame(chosen);
    }
}

void Game::renderCampaign()
{
    m_campaignScreen.draw(m_window);
}

// ============================================================
//  自定义模式画面
// ============================================================

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

void Game::renderCustomSetup()
{
    m_customScreen.draw(m_window);
}

// ============================================================
//  弹出菜单
// ============================================================

void Game::showBuildPopup(float x, float y)
{
    m_popupType = PopupType::Build;
    // 确保不超出屏幕
    m_popupPos.x = std::min(x, WINDOW_WIDTH - 160.0f);
    m_popupPos.y = std::min(y, WINDOW_HEIGHT - 130.0f);
}

void Game::showTowerPopup(float x, float y)
{
    m_popupType = PopupType::Tower;
    m_popupPos.x = std::min(x, WINDOW_WIDTH - 160.0f);
    m_popupPos.y = std::min(y, WINDOW_HEIGHT - 100.0f);
}

void Game::hidePopup()
{
    m_popupType = PopupType::None;
}

void Game::drawPopup()
{
    if (m_popupType == PopupType::None) return;

    float px = m_popupPos.x, py = m_popupPos.y;

    if (m_popupType == PopupType::Build)
    {
        TowerType types[] = {TowerType::Arrow, TowerType::Cannon, TowerType::Ice};
        TextKey nameKeys[] = {TextKey::Tower_Arrow, TextKey::Tower_Cannon, TextKey::Tower_Ice};

        for (int i = 0; i < 3; ++i)
        {
            TowerStats s = Tower::getStats(types[i]);
            sf::RectangleShape btn(sf::Vector2f(140, 34));
            btn.setPosition(px, py + i * 38);
            btn.setFillColor(s.color - sf::Color(60, 60, 60, 0));
            btn.setOutlineColor(sf::Color::White);
            btn.setOutlineThickness(1);
            m_window.draw(btn);

            sf::Text label;
            label.setFont(m_menuFont);
            label.setCharacterSize(15);
            label.setFillColor(sf::Color::White);
            label.setString(std::wstring(LangManager::get(nameKeys[i])) + L" $" + std::to_wstring(s.cost));
            label.setPosition(px + 5, py + i * 38 + 6);
            m_window.draw(label);
        }
    }
    else if (m_popupType == PopupType::Tower)
    {
        auto twr = m_popupTower.lock();
        if (!twr) { hidePopup(); return; }

        // 升级按钮
        {
            sf::RectangleShape btn(sf::Vector2f(140, 34));
            btn.setPosition(px, py);
            btn.setFillColor(twr->canUpgrade() ? sf::Color(50, 150, 50) : sf::Color(60, 60, 60));
            btn.setOutlineColor(sf::Color::White);
            btn.setOutlineThickness(1);
            m_window.draw(btn);

            sf::Text label;
            label.setFont(m_menuFont);
            label.setCharacterSize(15);
            label.setFillColor(sf::Color::White);
            if (twr->canUpgrade())
                label.setString(L"Lv" + std::to_wstring(twr->getLevel()) + L" → " + std::to_wstring(twr->getLevel() + 1) + L"  $" + std::to_wstring(twr->getUpgradeCost()));
            else
                label.setString(L"Lv" + std::to_wstring(twr->getLevel()) + L" MAX");
            label.setPosition(px + 5, py + 6);
            m_window.draw(label);
        }

        // 出售按钮
        {
            sf::RectangleShape btn(sf::Vector2f(140, 34));
            btn.setPosition(px, py + 38);
            btn.setFillColor(sf::Color(180, 60, 60));
            btn.setOutlineColor(sf::Color::White);
            btn.setOutlineThickness(1);
            m_window.draw(btn);

            sf::Text label;
            label.setFont(m_menuFont);
            label.setCharacterSize(15);
            label.setFillColor(sf::Color::White);
            label.setString(L"Sell $" + std::to_wstring(twr->getSellValue()));
            label.setPosition(px + 5, py + 38 + 6);
            m_window.draw(label);
        }
    }
}
