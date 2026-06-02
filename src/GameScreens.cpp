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
//  文本刷新
// ============================================================

void Game::refreshAllTexts()
{
    // 完全重建所有 UI（使用新语言文本和字体）
    initMenu();
    buildCharSelectUI();
    buildCharCreateUI();
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
        cfg.mapFile = params.mapFile;
        // 从路径推断群系
        if (cfg.mapFile.find("desert") != std::string::npos)
            cfg.biome = Biome::Desert;
        else if (cfg.mapFile.find("hell") != std::string::npos)
            cfg.biome = Biome::Hell;
        else if (cfg.mapFile.find("community") != std::string::npos)
            cfg.biome = Biome::Community;
        else
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
