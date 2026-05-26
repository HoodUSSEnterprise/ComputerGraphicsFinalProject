#pragma once

#include "Constants.h"
#include "Map.h"
#include "Tower.h"
#include "Enemy.h"
#include "Projectile.h"
#include "WaveManager.h"
#include "UI.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <fstream>

enum class GameState {
    Menu,
    Settings,
    CustomSetup,
    Playing,
    GameOver,
    GameWon
};

class Game {
public:
    Game();
    void run();

private:
    // ---- state ----
    void newGame();
    void newCustomGame();
    void saveGame();
    bool loadGame();
    void returnToMenu();

    // ---- events ----
    void processEvents();
    void handleResize();
    void processMenuEvents(const sf::Event& event);
    void processSettingsEvents(const sf::Event& event);
    void processCustomSetupEvents(const sf::Event& event);
    void processPlayingEvents(const sf::Event& event);

    // ---- update ----
    void update(float dt);

    // ---- render ----
    void render();
    void renderMenu();
    void renderSettings();
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
    bool  m_bgmOn;
    sf::Music m_bgm;
    void applyVolume();
    void initSettings();

    // ---- menu / settings UI ----
    sf::Font m_menuFont;
    sf::Text m_titleText;
    sf::Text m_subtitleText;

    struct MenuButton {
        sf::RectangleShape bg;
        sf::Text label;
        bool hovered = false;
    };
    std::vector<MenuButton> m_menuButtons;      // 主菜单按钮
    std::vector<MenuButton> m_settingsButtons;  // 设置界面按钮

    // 音量滑块
    sf::RectangleShape m_volTrack;
    sf::CircleShape    m_volKnob;
    bool m_draggingVol;

    // 语言切换标签
    sf::Text m_langLabel;
    sf::Text m_langValue;
    // BGM 标签
    sf::Text m_bgmLabel;

    void initMenu();
    void initCustomSetup();
    void buildSettingsUI();
    void loadMenuFont();
    void refreshAllTexts();
    void updateMenuHover(float mx, float my);
    int  getMenuButtonIndex(float mx, float my) const;
    int  getSettingsButtonIndex(float mx, float my) const;
    int  getCustomSetupButtonIndex(float mx, float my) const;

    // ---- 自定义模式参数 ----
    struct CustomParams {
        int waves = 5;
        int enemiesPerWave = 10;
        int startGold = 200;
        int startLives = 20;
        float speedMul = 1.0f;
        float hpMul = 1.0f;
    };
    CustomParams m_customParams;
    std::vector<MenuButton> m_customButtons;  // 自定义界面按钮
    void refreshCustomSetupTexts();
};
