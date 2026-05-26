// Game.cpp — 核心：构造、主循环、状态切换、事件/渲染分发
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>
#include <fstream>

Game::Game()
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + 100),
               "Tower Defense - SFML", sf::Style::Default),
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
    if (m_bgm.openFromFile("sound/bgm.mp3"))
    {
        m_bgm.setLoop(true);
        m_bgm.setVolume(m_volume);
        if (m_bgmOn) m_bgm.play();
    }
}

void Game::run()
{
    while (m_window.isOpen())
    {
        float dt = m_clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        processEvents();
        if (m_state == GameState::Playing) update(dt);
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
    m_towers.clear(); m_enemies.clear(); m_projectiles.clear();
    m_map = Map();
    m_map.loadFromFile(cfg.mapFile.c_str());
    static const char *biomeNames[] = {"grassland", "desert", "hell", "community"};
    int bIdx = static_cast<int>(cfg.biome);
    if (bIdx >= 0 && bIdx < 4) m_map.loadBiomeTextures(biomeNames[bIdx]);
    m_map.loadEndTextures();
    m_gold = cfg.startGold; m_lives = cfg.startLives;
    m_waveManager = WaveManager();
    m_waveManager.setCustomWaves(cfg.waveCount, cfg.baseEnemies, cfg.speedMul, cfg.hpMul);
    m_selectedTowerType = TowerType::Arrow;
    m_state = GameState::Playing;
    hidePopup();
}

void Game::saveGame()
{
    std::ofstream file("save.dat");
    if (!file) return;
    file << m_gold << '\n' << m_lives << '\n'
         << static_cast<int>(m_selectedTowerType) << '\n'
         << m_waveManager.getCurrentWave() << '\n' << m_towers.size() << '\n';
    for (const auto &t : m_towers)
        file << static_cast<int>(t->getType()) << ' ' << t->getPosition().x << ' ' << t->getPosition().y << '\n';
}

bool Game::loadGame()
{
    std::ifstream file("save.dat");
    if (!file) return false;
    int gold, lives, selType, wave;
    file >> gold >> lives >> selType >> wave;
    m_gold = gold; m_lives = lives;
    m_selectedTowerType = static_cast<TowerType>(selType);
    m_waveManager = WaveManager();
    for (int i = 0; i < wave; ++i) m_waveManager.startNextWave();
    size_t towerCount; file >> towerCount;
    m_towers.clear(); m_map = Map();
    for (size_t i = 0; i < towerCount; ++i)
    {
        int type; float x, y; file >> type >> x >> y;
        m_towers.push_back(std::make_shared<Tower>(static_cast<TowerType>(type), sf::Vector2f(x, y)));
        auto grid = m_map.worldToGrid(x, y);
        m_map.setTile(grid.x, grid.y, TileType::Blocked);
    }
    m_enemies.clear(); m_projectiles.clear();
    m_state = GameState::Playing;
    return true;
}

void Game::returnToMenu() { m_state = GameState::Menu; }

// ============================================================
//  事件分发
// ============================================================

void Game::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) { m_window.close(); return; }
        if (event.type == sf::Event::Resized) { handleResize(); continue; }
        switch (m_state)
        {
        case GameState::Menu:           processMenuEvents(event); break;
        case GameState::Settings:
            processSettingsEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) m_state = GameState::Menu;
            break;
        case GameState::CampaignSelect:
            processCampaignEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) m_state = GameState::Menu;
            break;
        case GameState::CustomSetup:
            processCustomSetupEvents(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) m_state = GameState::Menu;
            break;
        case GameState::Playing:        processPlayingEvents(event); break;
        case GameState::GameOver:
        case GameState::GameWon:
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) newGame();
                else if (event.key.code == sf::Keyboard::Escape) returnToMenu();
            } break;
        default: break;
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
    if (windowRatio > viewRatio) { vpH = viewH; vpW = vpH * windowRatio; }
    else                         { vpW = viewW; vpH = vpW / windowRatio; }
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
    case GameState::Menu:           renderMenu(); break;
    case GameState::Settings:       renderSettings(); break;
    case GameState::CampaignSelect: renderCampaign(); break;
    case GameState::CustomSetup:    renderCustomSetup(); break;
    case GameState::Playing:        renderPlaying(); break;
    case GameState::GameOver:
    case GameState::GameWon:        renderPlaying(); renderEndScreen(); break;
    default: break;
    }
    m_window.display();
}
