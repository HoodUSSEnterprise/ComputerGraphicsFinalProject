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
#include "PlayerData.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <fstream>

enum class GameState
{
    CharSelect,         // 角色选择/创建
    CharLoad,           // 加载已有角色
    Menu,               // 主菜单
    Settings,
    CampaignSelect,
    CustomSetup,
    Shop,               // 商店
    MapEditor,          // 地图编辑器
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
    void enterMenu();
    void enterMapEditor();       // 从角色选择进入主菜单
    void applyShopBonuses(const LevelConfig &cfg, int &gold, int &lives) const;

    // ---- events ----
    void processEvents();
    void handleResize();
    void processCharSelectEvents(const sf::Event &event);
    void processCharLoadEvents(const sf::Event &event);
    void processMenuEvents(const sf::Event &event);
    void processSettingsEvents(const sf::Event &event);
    void processCampaignEvents(const sf::Event &event);
    void processCustomSetupEvents(const sf::Event &event);
    void processShopEvents(const sf::Event &event);
    void processMapEditorEvents(const sf::Event &event);
    void processPlayingEvents(const sf::Event &event);

    // ---- update ----
    void update(float dt);

    // ---- render ----
    void render();
    void renderCharSelect();
    void renderCharLoad();
    void renderMenu();
    void renderSettings();
    void renderCampaign();
    void renderCustomSetup();
    void renderShop();
    void renderMapEditor();
    void renderPlaying();
    void renderEndScreen();
    void renderConfirmDialog();    // 确认覆盖存档对话框

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

    // 宝藏目标（点击后塔会攻击）
    sf::Vector2i m_treasureTarget = {-1, -1};

    // 地图编辑器
    TileType m_editGrid[MAP_COLS][MAP_ROWS] = {};
    int m_editMode = 1;
    std::wstring m_editorMsg;
    std::wstring m_editorFileName;
    bool m_editorEditingName = false;
    mutable sf::RectangleShape m_tileShape;

    // ---- 作弊码系统（罪恶都市风格） ----
    char m_cheatBuffer[CheatCode::BUFFER_SIZE] = {};
    int  m_cheatBufLen = 0;
    bool m_infiniteGold   = false;
    bool m_infiniteDamage = false;
    bool m_unlockAll      = false;  // 临时解锁所有关卡
    sf::Clock m_cheatMsgClock;
    sf::Text  m_cheatMsgText;
    void processCheatInput(sf::Uint32 unicode);
    void activateCheat(const std::string &code);
    void clearCheatBuffer();
    void killAllEnemies();
    void spawnBoss();
    void clearLevel();

    // ---- 暂停菜单 ----
    bool m_paused = false;
    struct PauseButton {
        sf::RectangleShape bg;
        sf::Text label;
        bool hovered = false;
    };
    std::vector<PauseButton> m_pauseButtons;
    sf::Text m_pauseTitle;
    void buildPauseMenu();
    void renderPauseMenu();
    void processPauseEvents(const sf::Event &event);

    // ---- data ----
    sf::RenderWindow m_window;
    sf::View m_view;
    GameState m_state;
    GameState m_stateBeforeSettings = GameState::Menu;  // 记录进入设置前的状态

    Map m_map;
    UI m_ui;

    std::vector<std::shared_ptr<Tower>> m_towers;
    std::vector<std::shared_ptr<Enemy>> m_enemies;
    std::vector<std::shared_ptr<Projectile>> m_projectiles;

    WaveManager m_waveManager;

    TowerType m_selectedTowerType;
    int m_gold;
    int m_lives;
    int m_currentCampaignIndex = -1;
    float m_waveCountdown = 0;  // 下一波倒计时

    sf::Clock m_clock;

    // ---- settings ----
    float m_volume;
    bool m_bgmOn;
    sf::Music m_bgm;
    void applyVolume();
    void initSettings();

    // ---- menu / settings UI ----
    sf::Font m_menuFont;
    sf::Texture m_buttonTex;
    sf::Texture m_infoTex;
    sf::Texture m_arrowLeftTex;
    sf::Texture m_arrowRightTex;
    sf::Text m_titleText;

    // 背景图片
    std::vector<sf::Texture> m_bgTextures;
    std::vector<sf::Sprite> m_bgSprites;
    int m_bgIndex = 0;
    void loadBackgrounds();
    void drawBackground();
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

    // ---- 角色系统 ----
    PlayerData m_playerData;
    bool m_hasCharacter = false;
    // 角色选择界面
    std::vector<PlayerData> m_charList;
    std::string m_newCharName;
    sf::Text m_charTitleText;
    sf::Text m_charNameInput;      // 显示正在输入的名字
    sf::Text m_charHintText;
    struct CharButton {
        sf::RectangleShape bg;
        sf::Text nameText;
        sf::Text infoText;
        bool hovered = false;
    };
    std::vector<CharButton> m_charButtons;
    sf::RectangleShape m_newCharBtn;
    sf::Text m_newCharBtnLabel;
    sf::RectangleShape m_loadCharBtn;
    sf::Text m_loadCharBtnLabel;
    bool m_showCharList = false;  // 点击"加载角色"后显示列表
    sf::RectangleShape m_confirmCharBtn;
    sf::Text m_confirmCharBtnLabel;
    void refreshCharList();
    void buildCharSelectUI();

    // ---- 商店系统 ----
    struct ShopButton {
        sf::RectangleShape bg;
        sf::Text nameText;
        sf::Text levelText;
        sf::Text costText;
        sf::RectangleShape buyBtn;
        sf::Text buyLabel;
        bool hovered = false;
    };
    std::vector<ShopButton> m_shopButtons;
    sf::Text m_shopTitleText;
    sf::Text m_shopGoldText;
    sf::Text m_shopBackHint;
    void buildShopUI();
    void refreshShopTexts();

    // ---- 确认对话框 ----
    bool m_showConfirm = false;
    sf::Text m_confirmTitleText;
    sf::Text m_confirmMsgText;
    sf::RectangleShape m_confirmYesBtn;
    sf::Text m_confirmYesLabel;
    sf::RectangleShape m_confirmNoBtn;
    sf::Text m_confirmNoLabel;
    void buildConfirmUI();
};
