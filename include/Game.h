#pragma once

#include "Constants.h"
#include "Map.h"
#include "Tower.h"
#include "Enemy.h"
#include "Projectile.h"
#include "WaveManager.h"
#include "UI.h"
#include "LevelData.h"
#include "CampaignScreen.h"
#include "CustomScreen.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <fstream>

enum class GameState
{
    Menu,
    Settings,
    CampaignSelect,
    CustomSetup,
    Playing,
    GameOver,
    GameWon
};

class Game
{
public:
    Game();
    void run();

private:
    // ---- state ----
    void newGame(const LevelConfig &cfg);
    void newGame();
    void saveGame();
    bool loadGame();
    void returnToMenu();

    // ---- events ----
    void processEvents();
    void handleResize();
    void processMenuEvents(const sf::Event &event);
    void processSettingsEvents(const sf::Event &event);
    void processCampaignEvents(const sf::Event &event);
    void processCustomSetupEvents(const sf::Event &event);
    void processPlayingEvents(const sf::Event &event);

    // ---- update ----
    void update(float dt);

    // ---- render ----
    void render();
    void renderMenu();
    void renderSettings();
    void renderCampaign();
    void renderCustomSetup();
    void renderPlaying();
    void renderEndScreen();

    // ---- gameplay ----
    void handleTowerPlacement(float x, float y);
    void handleTowerSelection(float x, float y);
    void updateTowers(float dt);
    void updateProjectiles(float dt);
    void updateEnemies(float dt);
    void checkProjectileCollisions();
    void towerFindTargets();
    void handleSellTower(float x, float y);
    void handleTowerUpgrade(float x, float y);

    // ---- 弹出菜单 ----
    enum class PopupType { None, Build, Tower };
    PopupType m_popupType = PopupType::None;
    sf::Vector2f m_popupPos;
    sf::Vector2i m_popupGrid;
    std::weak_ptr<Tower> m_popupTower;
    void showBuildPopup(float x, float y, int gx, int gy);
    void showTowerPopup(float x, float y);
    void hidePopup();
    void drawPopup();

    // ---- 作弊码系统（罪恶都市风格） ----
    char m_cheatBuffer[CheatCode::BUFFER_SIZE] = {};
    int  m_cheatBufLen = 0;
    bool m_infiniteGold   = false;
    bool m_infiniteDamage = false;
    sf::Clock m_cheatMsgClock;
    sf::Text  m_cheatMsgText;
    void processCheatInput(sf::Uint32 unicode);
    void activateCheat(const std::string &code);
    void clearCheatBuffer();
    void killAllEnemies();
    void spawnBoss();
    void clearLevel();

    // ---- data ----
    sf::RenderWindow m_window;
    sf::View m_view;
    GameState m_state;

    Map m_map;
    UI m_ui;

    std::vector<std::shared_ptr<Tower>> m_towers;
    std::vector<std::shared_ptr<Enemy>> m_enemies;
    std::vector<std::shared_ptr<Projectile>> m_projectiles;

    WaveManager m_waveManager;

    TowerType m_selectedTowerType;
    int m_gold;
    int m_lives;

    sf::Clock m_clock;

    // ---- settings ----
    float m_volume;
    bool m_bgmOn;
    sf::Music m_bgm;
    void applyVolume();
    void initSettings();

    // ---- menu / settings UI ----
    sf::Font m_menuFont;
    sf::Text m_titleText;
    sf::Text m_subtitleText;

    struct MenuButton
    {
        sf::RectangleShape bg;
        sf::Text label;
        bool hovered = false;
    };
    std::vector<MenuButton> m_menuButtons;     // 主菜单按钮
    std::vector<MenuButton> m_settingsButtons; // 设置界面按钮

    // 音量滑块
    sf::RectangleShape m_volTrack;
    sf::CircleShape m_volKnob;
    bool m_draggingVol;

    // 语言切换标签
    sf::Text m_langLabel;
    sf::Text m_langValue;
    // BGM 标签
    sf::Text m_bgmLabel;

    void initMenu();
    void buildSettingsUI();
    void loadMenuFont();
    void refreshAllTexts();
    void updateMenuHover(float mx, float my);
    int getMenuButtonIndex(float mx, float my) const;
    int getSettingsButtonIndex(float mx, float my) const;

    // ---- 战役 & 自定义模式 ----
    CampaignScreen m_campaignScreen;
    CustomScreen m_customScreen;
};
