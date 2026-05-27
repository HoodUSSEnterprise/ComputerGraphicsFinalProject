// Game.cpp — 核心：构造、主循环、状态切换、事件/渲染分发
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>

Game::Game()
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + 100),
               "Tower Defense - SFML", sf::Style::Default),
      m_state(GameState::CharSelect), m_selectedTowerType(TowerType::Arrow), m_gold(200), m_lives(20),
      m_volume(80.0f), m_bgmOn(true), m_draggingVol(false)
{
    m_window.setFramerateLimit(60);
    m_view = sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    m_window.setView(m_view);
    LangManager::loadLanguage("assets/lang_zh.json");
    m_ui.reloadFont();
    m_campaignScreen.reloadFont();
    m_campaignScreen.refreshTexts();
    m_customScreen.reloadFont();
    m_customScreen.refreshTexts();
    initMenu();
    initSettings();
    Enemy::loadTextures();
    Tower::loadTextures();
    m_buttonTex.loadFromFile("textures/button.png");
    m_infoTex.loadFromFile("textures/information.png");
    m_arrowLeftTex.loadFromFile("textures/ArrowLeft.png");
    m_arrowRightTex.loadFromFile("textures/ArrowRight.png");
    if (m_bgm.openFromFile("sound/bgm.mp3"))
    {
        m_bgm.setLoop(true);
        m_bgm.setVolume(m_volume);
        if (m_bgmOn)
            m_bgm.play();
    }
    // 初始化角色选择界面
    buildCharSelectUI();
    refreshCharList();
    // 初始化确认对话框UI
    buildConfirmUI();
    // 初始化暂停菜单
    buildPauseMenu();
    // 加载背景
    loadBackgrounds();
}

void Game::loadBackgrounds()
{
    m_bgTextures.clear();
    m_bgSprites.clear();
    m_bgIndex = 0;

    namespace fs = std::filesystem;
    std::string dir = "textures/background";
    std::error_code ec;
    if (!fs::exists(dir, ec)) { fs::create_directories(dir, ec); return; }

    // 先加载所有纹理
    for (const auto &entry : fs::directory_iterator(dir, ec))
    {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") continue;

        sf::Texture tex;
        if (tex.loadFromFile(entry.path().string()))
        {
            tex.setSmooth(true);
            m_bgTextures.push_back(std::move(tex));
            std::cout << "[BG] Loaded: " << entry.path().filename() << std::endl;
        }
    }

    // 纹理全加载完后再创建精灵（避免 vector 扩容导致指针失效）
    for (auto &tex : m_bgTextures)
    {
        sf::Sprite spr(tex);
        auto sz = tex.getSize();
        float sx = static_cast<float>(WINDOW_WIDTH) / sz.x;
        float sy = static_cast<float>(WINDOW_HEIGHT) / sz.y;
        spr.setScale(sx, sy);
        m_bgSprites.push_back(std::move(spr));
    }
    std::cout << "[BG] Total backgrounds: " << m_bgTextures.size() << std::endl;
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
            update(dt);
        render();
    }
}

// ============================================================
//  状态切换
// ============================================================

void Game::newGame()
{
    static LevelConfig defaultCfg = getCampaignLevels()[0];
    newGame(defaultCfg);
}

void Game::newGame(const LevelConfig &cfg)
{
    m_towers.clear();
    m_enemies.clear();
    m_projectiles.clear();
    m_map.loadFromFile(cfg.mapFile.c_str());
    static const char *biomeNames[] = {"grassland", "desert", "hell", "community"};
    int bIdx = static_cast<int>(cfg.biome);
    if (bIdx >= 0 && bIdx < 4)
    {
        m_map.loadBiomeTextures(biomeNames[bIdx]);
    }
    m_map.loadEndTextures();
    m_map.loadBirthTexture();
    m_map.loadTreasureTextures();
    m_map.initTreasureHP();

    // 应用商店加成
    applyShopBonuses(cfg, m_gold, m_lives);

    m_waveManager = WaveManager();
    m_waveManager.setCustomWaves(cfg.waveCount, cfg.baseEnemies, cfg.speedMul, cfg.hpMul);
    m_selectedTowerType = TowerType::Arrow;
    m_state = GameState::Playing;
    hidePopup();
    // 重置作弊状态
    m_infiniteGold = false;
    m_infiniteDamage = false;
    clearCheatBuffer();
    m_waveCountdown = 10.0f;  // 第一波10秒倒计时

    // 记录当前战役关卡索引（用于通关后解锁下一关）
    auto levels = getCampaignLevels();
    m_currentCampaignIndex = -1;
    for (size_t i = 0; i < levels.size(); ++i)
    {
        if (levels[i].id == cfg.id)
        {
            m_currentCampaignIndex = static_cast<int>(i);
            break;
        }
    }
}

void Game::saveGame()
{
    std::ofstream file("save.dat");
    if (!file)
        return;
    file << m_gold << '\n'
         << m_lives << '\n'
         << static_cast<int>(m_selectedTowerType) << '\n'
         << m_waveManager.getCurrentWave() << '\n'
         << m_towers.size() << '\n';
    for (const auto &t : m_towers)
        file << static_cast<int>(t->getType()) << ' ' << t->getPosition().x << ' ' << t->getPosition().y << '\n';
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
    m_waveManager = WaveManager();
    for (int i = 0; i < wave; ++i)
        m_waveManager.startNextWave();
    size_t towerCount;
    file >> towerCount;
    m_towers.clear();
    for (size_t i = 0; i < towerCount; ++i)
    {
        int type;
        float x, y;
        file >> type >> x >> y;
        m_towers.push_back(std::make_shared<Tower>(static_cast<TowerType>(type), sf::Vector2f(x, y)));
        auto grid = m_map.worldToGrid(x, y);
        m_map.setTile(grid.x, grid.y, TileType::Blocked);
    }
    m_enemies.clear();
    m_projectiles.clear();
    m_state = GameState::Playing;
    return true;
}

void Game::returnToMenu() { m_state = GameState::Menu; }

void Game::enterMenu()
{
    m_state = GameState::Menu;
    // 重新加载角色信息以刷新菜单文本
}

void Game::applyShopBonuses(const LevelConfig &cfg, int &gold, int &lives) const
{
    gold = cfg.startGold + m_playerData.getStartGoldBonus();
    lives = cfg.startLives + m_playerData.getLivesBonus();
}

// ============================================================
//  事件分发
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
        case GameState::CharSelect:
            processCharSelectEvents(event);
            break;
        case GameState::CharLoad:
            processCharLoadEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                m_state = GameState::CharSelect;
            break;
        case GameState::Menu:
            processMenuEvents(event);
            break;
        case GameState::Settings:
            processSettingsEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                m_state = m_stateBeforeSettings;
                m_stateBeforeSettings = GameState::Menu;
            }
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
        case GameState::Shop:
            processShopEvents(event);
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
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f worldPos = m_window.mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                float mx = worldPos.x, my = worldPos.y;

                // "下一关" / "返回主页" 按钮区域（适配中英文布局）
                if (m_state == GameState::GameWon && m_hasCharacter && m_currentCampaignIndex >= 0)
                {
                    auto levels = getCampaignLevels();
                    bool hasNext = (m_currentCampaignIndex + 1 < m_playerData.unlockedLevels &&
                                    m_currentCampaignIndex + 1 < static_cast<int>(levels.size()));
                    bool zh = (LangManager::currentLangName() == "zh");
                    float btnW = zh ? 180.0f : 160.0f;
                    float gap = zh ? 20.0f : 10.0f;

                    // 下一关按钮
                    if (hasNext && mx >= WINDOW_WIDTH / 2.0f - btnW - gap / 2 &&
                        mx <= WINDOW_WIDTH / 2.0f - gap / 2 &&
                        my >= WINDOW_HEIGHT / 2.0f + 5 && my <= WINDOW_HEIGHT / 2.0f + 49)
                    {
                        newGame(levels[m_currentCampaignIndex + 1]);
                        return;
                    }

                    // 返回主页按钮
                    if (mx >= WINDOW_WIDTH / 2.0f + gap / 2 &&
                        mx <= WINDOW_WIDTH / 2.0f + btnW + gap / 2 &&
                        my >= WINDOW_HEIGHT / 2.0f + 5 && my <= WINDOW_HEIGHT / 2.0f + 49)
                    {
                        returnToMenu();
                        return;
                    }
                }
                else
                {
                    // 点击任意位置返回
                    returnToMenu();
                }
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
    float viewW = WINDOW_WIDTH, viewH = WINDOW_HEIGHT + 100;
    float windowRatio = winW / winH, viewRatio = viewW / viewH;
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

// ============================================================
//  渲染分发
// ============================================================

void Game::render()
{
    m_window.clear(sf::Color(20, 20, 30));
    m_window.setView(m_view);
    switch (m_state)
    {
    case GameState::CharSelect:
        renderCharSelect();
        break;
    case GameState::CharLoad:
        renderCharLoad();
        break;
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
    case GameState::Shop:
        renderShop();
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
