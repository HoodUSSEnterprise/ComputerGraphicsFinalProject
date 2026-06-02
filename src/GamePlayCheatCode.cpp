// GamePlay.cpp — Game 的游戏逻辑实现
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

// ============================================================
//  作弊码系统（罪恶都市风格键盘输入）
// ============================================================

void Game::processCheatInput(sf::Uint32 unicode)
{
    // 只接受可打印 ASCII 字母，自动转小写
    if (unicode >= 'A' && unicode <= 'Z')
    {
        unicode = unicode + ('a' - 'A');
    }
    if (unicode < 'a' || unicode > 'z')
    {
        // 回车键清空缓冲区
        if (unicode == '\r' || unicode == '\n')
            clearCheatBuffer();
        return;
    }

    if (m_cheatBufLen < CheatCode::BUFFER_SIZE - 1)
    {
        m_cheatBuffer[m_cheatBufLen++] = static_cast<char>(unicode);
        m_cheatBuffer[m_cheatBufLen] = '\0';
    }
    else
    {
        // 缓冲区满，左移一位再追加
        for (int i = 0; i < CheatCode::BUFFER_SIZE - 2; ++i)
            m_cheatBuffer[i] = m_cheatBuffer[i + 1];
        m_cheatBuffer[CheatCode::BUFFER_SIZE - 2] = static_cast<char>(unicode);
    }

    // 检查是否匹配任何作弊码
    std::string buf(m_cheatBuffer);
    if (buf.find(CheatCode::InfiniteGold) != std::string::npos)
        activateCheat(CheatCode::InfiniteGold);
    else if (buf.find(CheatCode::InfiniteDamage) != std::string::npos)
        activateCheat(CheatCode::InfiniteDamage);
    else if (buf.find(CheatCode::KillAll) != std::string::npos)
        activateCheat(CheatCode::KillAll);
    else if (buf.find(CheatCode::ClearLevel) != std::string::npos)
        activateCheat(CheatCode::ClearLevel);
    else if (buf.find(CheatCode::SpawnBoss) != std::string::npos)
        activateCheat(CheatCode::SpawnBoss);
    else if (buf.find(CheatCode::UnlockAll) != std::string::npos)
        activateCheat(CheatCode::UnlockAll);
    else if (buf.find(CheatCode::LastWave) != std::string::npos)
        activateCheat(CheatCode::LastWave);
}

void Game::activateCheat(const std::string &code)
{
    clearCheatBuffer();

    std::string msg;
    if (code == CheatCode::InfiniteGold)
    {
        m_infiniteGold = !m_infiniteGold;
        msg = m_infiniteGold ? "Infinite Gold ON!" : "Infinite Gold OFF!";
    }
    else if (code == CheatCode::InfiniteDamage)
    {
        m_infiniteDamage = !m_infiniteDamage;
        msg = m_infiniteDamage ? "Infinite Damage ON!" : "Infinite Damage OFF!";
    }
    else if (code == CheatCode::KillAll)
    {
        killAllEnemies();
        msg = "All Enemies Killed!";
    }
    else if (code == CheatCode::ClearLevel)
    {
        clearLevel();
        msg = "Level Cleared!";
    }
    else if (code == CheatCode::SpawnBoss)
    {
        spawnBoss();
        msg = "BOSS Spawned!";
    }
    else if (code == CheatCode::UnlockAll)
    {
        m_unlockAll = !m_unlockAll;
        if (m_unlockAll && m_hasCharacter)
        {
            auto levels = getCampaignLevels();
            m_playerData.unlockedLevels = static_cast<int>(levels.size());
        }
        msg = m_unlockAll ? "All Levels Unlocked!" : "Levels Locked Again!";
    }
    else if (code == CheatCode::LastWave)
    {
        m_waveManager.skipToLastWave();
        msg = "Skipped to Last Wave!";
    }

    // 显示作弊提示消息（金色大字）
    m_cheatMsgText.setFont(m_menuFont);
    m_cheatMsgText.setString(msg);
    m_cheatMsgText.setCharacterSize(28);
    m_cheatMsgText.setFillColor(sf::Color(255, 215, 0));
    m_cheatMsgText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_cheatMsgText.getLocalBounds();
    m_cheatMsgText.setOrigin(tb.width / 2, tb.height / 2);
    m_cheatMsgText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 60);
    m_cheatMsgClock.restart();

    // 同时在 UI 消息栏也显示
    std::wstring wmsg(msg.begin(), msg.end());
    m_ui.showMessage(wmsg);
}

void Game::clearCheatBuffer()
{
    m_cheatBufLen = 0;
    m_cheatBuffer[0] = '\0';
}

void Game::killAllEnemies()
{
    for (auto &enemy : m_enemies)
    {
        if (!enemy->isDead() && !enemy->hasReachedEnd())
            enemy->takeDamage(99999.0f);
    }
    // 清理死亡敌人
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
                       [](const std::shared_ptr<Enemy> &e)
                       { return e->isDead(); }),
        m_enemies.end());
}

void Game::spawnBoss()
{
    const auto &waypoints = m_map.getWaypoints();
    if (waypoints.empty())
        return;

    int variant = Enemy::getRandomBoss();
    auto boss = std::make_shared<Enemy>(0, 80.0f, 300.0f, 50, variant);
    boss->setPosition(waypoints[0].pos);
    m_enemies.push_back(boss);
}

void Game::clearLevel()
{
    killAllEnemies();
    m_waveManager.forceAllComplete();
    // 不直接设 GameWon，让 update() 检测到通关并执行解锁逻辑
}