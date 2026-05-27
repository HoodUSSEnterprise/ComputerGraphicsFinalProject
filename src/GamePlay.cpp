// GamePlay.cpp — Game 的游戏逻辑实现
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

// ============================================================
//  游戏事件处理
// ============================================================

void Game::processPlayingEvents(const sf::Event &event)
{
    // 暂停状态下特殊处理
    if (m_paused)
    {
        processPauseEvents(event);
        return;
    }

    // 作弊码输入：捕获键盘字符（罪恶都市风格）
    if (event.type == sf::Event::TextEntered)
    {
        processCheatInput(event.text.unicode);
        return;
    }

    if (event.type == sf::Event::KeyPressed)
    {
        switch (event.key.code)
        {
        case sf::Keyboard::Space:
            if (m_waveManager.canStartWave())
                m_waveManager.startNextWave();
            break;
        case sf::Keyboard::Escape:
            m_paused = true;
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
        if (m_ui.isClickOnUI(mx, my))
        {
            if (m_ui.isStartWaveClicked(mx, my))
            {
                if (m_waveManager.canStartWave())
                {
                    m_waveManager.startNextWave();
                    int waveIdx = m_waveManager.getCurrentWave();
                    if (waveIdx > 1)
                        m_gold += 50 + (waveIdx - 1) * 10;
                    m_ui.showMessage(std::wstring(LangManager::get(TextKey::Wave)) + L" " + std::to_wstring(waveIdx) + L" " + LangManager::get(TextKey::Msg_WaveStarted));
                }
            }
            hidePopup();
            return;
        }

        if (m_popupType == PopupType::Build)
        {
            float px = m_popupPos.x, py = m_popupPos.y;
            int gx = m_popupGrid.x, gy = m_popupGrid.y;
            for (int i = 0; i < 3; ++i)
            {
                if (mx >= px && mx <= px + 140 && my >= py + i * 38 && my <= py + i * 38 + 34)
                {
                    TowerType tt = static_cast<TowerType>(i);
                    int baseCost = Tower::getStats(tt).cost;
                    int cost = baseCost;
                    if (m_hasCharacter)
                    {
                        float discount = m_playerData.getTowerDiscount();
                        cost = static_cast<int>(baseCost * (1.0f - discount));
                        if (cost < 1) cost = 1;
                    }
                    if (!m_infiniteGold && m_gold < cost)
                    {
                        m_ui.showMessage(LangManager::get(TextKey::Msg_NoGold));
                    }
                    else if (!m_map.canPlaceTower(gx, gy))
                    {
                        m_ui.showMessage(LangManager::get(TextKey::Msg_CannotPlace));
                    }
                    else
                    {
                        sf::Vector2f center = m_map.gridToWorld(gx, gy);
                        m_towers.push_back(std::make_shared<Tower>(tt, center));
                        if (!m_infiniteGold) m_gold -= cost;
                        m_map.setTile(gx, gy, TileType::Blocked);
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
                if (mx >= px && mx <= px + 140 && my >= py && my <= py + 34)
                {
                    if (twr->canUpgrade() && (m_infiniteGold || m_gold >= twr->getUpgradeCost()))
                    { if (!m_infiniteGold) m_gold -= twr->getUpgradeCost(); twr->upgrade(); }
                    hidePopup(); return;
                }
                if (mx >= px && mx <= px + 140 && my >= py + 38 && my <= py + 72)
                {
                    int refund = twr->getSellValue();
                    m_gold += refund;
                    auto grid = m_map.worldToGrid(twr->getPosition().x, twr->getPosition().y);
                    m_map.setTile(grid.x, grid.y, TileType::Grass);
                    m_towers.erase(std::remove_if(m_towers.begin(), m_towers.end(),
                        [&](auto &t) { return t.get() == twr.get(); }), m_towers.end());
                    m_ui.showMessage(LangManager::get(TextKey::Msg_Sold) + std::to_wstring(refund));
                    hidePopup(); return;
                }
            }
            hidePopup(); return;
        }

        auto grid = m_map.worldToGrid(mx, my);
        if (m_map.getTile(grid.x, grid.y) == TileType::Grass)
        {
            showBuildPopup(mx, my, grid.x, grid.y);
        }
        else
        {
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
    if (m_paused) return;  // 暂停时不更新游戏逻辑

    updateEnemies(dt);
    updateTowers(dt);
    towerFindTargets();
    updateProjectiles(dt);
    checkProjectileCollisions();
    m_waveManager.update(dt, m_enemies, m_map.getWaypoints());

    if (m_waveManager.areAllWavesComplete() && m_enemies.empty())
    {
        // 通关：累计金币，解锁下一关，保存进度
        if (m_state != GameState::GameWon)
        {
            if (m_hasCharacter && m_currentCampaignIndex >= 0)
            {
                m_playerData.totalGold += m_gold;
                // 只有当前关卡是最后解锁的关卡时，才解锁下一关
                auto levels = getCampaignLevels();
                int totalLevels = static_cast<int>(levels.size());
                if (m_currentCampaignIndex + 1 == m_playerData.unlockedLevels &&
                    m_playerData.unlockedLevels < totalLevels)
                {
                    m_playerData.unlockedLevels++;
                }
                m_playerData.save(PlayerData::makeSavePath(m_playerData.name));
            }
        }
        m_state = GameState::GameWon;
    }

    m_ui.update(dt, m_gold, m_lives,
                m_waveManager.getCurrentWave(),
                m_waveManager.getTotalWaves(),
                m_selectedTowerType);

    if (m_lives <= 0)
        m_state = GameState::GameOver;
}

// ============================================================
//  渲染
// ============================================================

void Game::renderPlaying()
{
    m_map.draw(m_window);
    for (const auto &tower : m_towers) tower->draw(m_window);
    for (const auto &proj : m_projectiles) proj->draw(m_window);
    for (const auto &enemy : m_enemies) enemy->draw(m_window);
    m_ui.draw(m_window);
    drawPopup();

    // 作弊码激活提示
    if (m_cheatMsgClock.getElapsedTime().asSeconds() < 3.0f)
    {
        m_window.draw(m_cheatMsgText);
    }

    // 暂停菜单
    if (m_paused)
        renderPauseMenu();
}

void Game::renderEndScreen()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // 对话框背景
    float dlgW = zh ? 460.0f : 420.0f;
    sf::RectangleShape dlgBg(sf::Vector2f(dlgW, 240));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - dlgW / 2, WINDOW_HEIGHT / 2.0f - 120);
    dlgBg.setFillColor(sf::Color(25, 25, 45));
    dlgBg.setOutlineColor(m_state == GameState::GameWon ? sf::Color(255, 215, 0) : sf::Color(255, 80, 80));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    // 标题
    sf::Text title;
    title.setFont(m_menuFont);
    title.setCharacterSize(zh ? 40 : 44);
    title.setStyle(sf::Text::Bold);
    if (m_state == GameState::GameWon)
    { title.setString(LangManager::get(TextKey::Victory)); title.setFillColor(sf::Color::Yellow); }
    else
    { title.setString(LangManager::get(TextKey::GameOver)); title.setFillColor(sf::Color::Red); }
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2, tb.height / 2);
    title.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 70);
    m_window.draw(title);

    // 通关时显示"下一关"和"返回主页"按钮
    if (m_state == GameState::GameWon && m_hasCharacter && m_currentCampaignIndex >= 0)
    {
        auto levels = getCampaignLevels();
        bool hasNext = (m_currentCampaignIndex + 1 < m_playerData.unlockedLevels &&
                        m_currentCampaignIndex + 1 < static_cast<int>(levels.size()));

        float btnW = zh ? 180.0f : 160.0f;
        float gap = zh ? 20.0f : 10.0f;

        // 下一关按钮
        {
            sf::RectangleShape btn(sf::Vector2f(btnW, 44));
            btn.setPosition(WINDOW_WIDTH / 2.0f - btnW - gap / 2, WINDOW_HEIGHT / 2.0f + 5);
            btn.setFillColor(hasNext ? sf::Color(40, 100, 40) : sf::Color(50, 50, 50));
            btn.setOutlineColor(hasNext ? sf::Color(100, 200, 100) : sf::Color(80, 80, 80));
            btn.setOutlineThickness(2);
            m_window.draw(btn);

            sf::Text btnText;
            btnText.setFont(m_menuFont);
            btnText.setString(LangManager::get(TextKey::NextLevel));
            btnText.setCharacterSize(zh ? 18 : 20);
            btnText.setFillColor(hasNext ? sf::Color::White : sf::Color(120, 120, 120));
            sf::FloatRect bb = btnText.getLocalBounds();
            btnText.setOrigin(bb.width / 2, bb.height / 2);
            btnText.setPosition(WINDOW_WIDTH / 2.0f - btnW / 2 - gap / 2, WINDOW_HEIGHT / 2.0f + 27);
            m_window.draw(btnText);
        }

        // 返回主页按钮
        {
            sf::RectangleShape btn(sf::Vector2f(btnW, 44));
            btn.setPosition(WINDOW_WIDTH / 2.0f + gap / 2, WINDOW_HEIGHT / 2.0f + 5);
            btn.setFillColor(sf::Color(50, 50, 70));
            btn.setOutlineColor(sf::Color(100, 100, 140));
            btn.setOutlineThickness(2);
            m_window.draw(btn);

            sf::Text btnText;
            btnText.setFont(m_menuFont);
            btnText.setString(LangManager::get(TextKey::BackToMenu));
            btnText.setCharacterSize(zh ? 18 : 20);
            btnText.setFillColor(sf::Color::White);
            sf::FloatRect bb = btnText.getLocalBounds();
            btnText.setOrigin(bb.width / 2, bb.height / 2);
            btnText.setPosition(WINDOW_WIDTH / 2.0f + btnW / 2 + gap / 2, WINDOW_HEIGHT / 2.0f + 27);
            m_window.draw(btnText);
        }
    }
    else if (m_state == GameState::GameOver)
    {
        sf::Text hint;
        hint.setFont(m_menuFont);
        hint.setCharacterSize(zh ? 20 : 22);
        hint.setFillColor(sf::Color::White);
        hint.setString(std::wstring(LangManager::get(TextKey::PressR)) + L"    |    " + std::wstring(LangManager::get(TextKey::EscToMenu)));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2, hb.height / 2);
        hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 30);
        m_window.draw(hint);
    }
    else
    {
        sf::Text hint;
        hint.setFont(m_menuFont);
        hint.setCharacterSize(zh ? 20 : 22);
        hint.setFillColor(sf::Color::White);
        hint.setString(LangManager::get(TextKey::BackToMenu));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2, hb.height / 2);
        hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 30);
        m_window.draw(hint);
    }
}

// ============================================================
//  游戏逻辑
// ============================================================

void Game::updateTowers(float dt) { for (auto &tower : m_towers) tower->update(dt); }
void Game::updateProjectiles(float dt) { for (auto &proj : m_projectiles) proj->update(dt); }

void Game::updateEnemies(float dt)
{
    const auto &waypoints = m_map.getWaypoints();
    for (auto &enemy : m_enemies) enemy->update(dt, waypoints);
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [this](const std::shared_ptr<Enemy> &e) {
                if (e->hasReachedEnd()) { m_lives--; return true; }
                return e->isDead();
            }), m_enemies.end());
}

void Game::towerFindTargets()
{
    float dmgMul = 1.0f;
    float rangeMul = 1.0f;
    float frMul = 1.0f;
    if (m_hasCharacter)
    {
        dmgMul = 1.0f + m_playerData.getDamageBoost();
        rangeMul = 1.0f + m_playerData.getRangeBoost();
        frMul = 1.0f + m_playerData.getFireRateBoost();
    }

    for (auto &tower : m_towers)
    {
        if (!tower->canFire()) continue;
        std::shared_ptr<Enemy> bestTarget = nullptr;
        float range = tower->getRange() * rangeMul;
        float bestDist = range;
        for (auto &enemy : m_enemies)
        {
            if (enemy->isDead() || enemy->hasReachedEnd()) continue;
            float dx = enemy->getPosition().x - tower->getPosition().x;
            float dy = enemy->getPosition().y - tower->getPosition().y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= range && (!bestTarget || dist < bestDist))
            { bestTarget = enemy; bestDist = dist; }
        }
        if (bestTarget)
        {
            m_projectiles.push_back(std::make_shared<Projectile>(
                tower->getPosition(), bestTarget, tower->getDamage() * dmgMul, 300.0f, tower->getType()));
            tower->resetFireTimer();
            if (frMul > 1.0f)
                tower->applyFireRateBoost(frMul);
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
                float dmg = m_infiniteDamage ? 99999.0f : proj->getDamage();
                target->takeDamage(dmg);
                if (proj->getTowerType() == TowerType::Ice) target->applySlow(0.4f, 2.0f);
                if (target->isDead()) m_gold += target->getReward();
            }
        }
    }
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::shared_ptr<Projectile> &p) { return p->hasHit(); }),
        m_projectiles.end());
}

// ============================================================
//  弹出菜单
// ============================================================

void Game::showBuildPopup(float x, float y, int gx, int gy)
{
    m_popupType = PopupType::Build;
    m_popupPos.x = std::min(x, WINDOW_WIDTH - 160.0f);
    m_popupPos.y = std::min(y, WINDOW_HEIGHT - 130.0f);
    m_popupGrid.x = gx;
    m_popupGrid.y = gy;
}

void Game::showTowerPopup(float x, float y)
{
    m_popupType = PopupType::Tower;
    m_popupPos.x = std::min(x, WINDOW_WIDTH - 160.0f);
    m_popupPos.y = std::min(y, WINDOW_HEIGHT - 100.0f);
}

void Game::hidePopup() { m_popupType = PopupType::None; }

void Game::drawPopup()
{
    if (m_popupType == PopupType::None) return;
    float px = m_popupPos.x, py = m_popupPos.y;

    if (m_popupType == PopupType::Build)
    {
        // 高亮目标格子
        float hx = m_popupGrid.x * TILE_SIZE;
        float hy = m_popupGrid.y * TILE_SIZE;
        sf::RectangleShape highlight(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        highlight.setPosition(hx, hy);
        highlight.setFillColor(sf::Color(255, 255, 0, 80));
        highlight.setOutlineColor(sf::Color(255, 255, 0, 180));
        highlight.setOutlineThickness(2);
        m_window.draw(highlight);

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

        {
            sf::RectangleShape btn(sf::Vector2f(140, 34));
            btn.setPosition(px, py);
            btn.setFillColor(twr->canUpgrade() ? sf::Color(50, 150, 50) : sf::Color(60, 60, 60));
            btn.setOutlineColor(sf::Color::White); btn.setOutlineThickness(1);
            m_window.draw(btn);
            sf::Text label;
            label.setFont(m_menuFont); label.setCharacterSize(15); label.setFillColor(sf::Color::White);
            if (twr->canUpgrade())
                label.setString(L"Lv" + std::to_wstring(twr->getLevel()) + L" → " + std::to_wstring(twr->getLevel() + 1) + L"  $" + std::to_wstring(twr->getUpgradeCost()));
            else
                label.setString(L"Lv" + std::to_wstring(twr->getLevel()) + L" MAX");
            label.setPosition(px + 5, py + 6);
            m_window.draw(label);
        }
        {
            sf::RectangleShape btn(sf::Vector2f(140, 34));
            btn.setPosition(px, py + 38);
            btn.setFillColor(sf::Color(180, 60, 60));
            btn.setOutlineColor(sf::Color::White); btn.setOutlineThickness(1);
            m_window.draw(btn);
            sf::Text label;
            label.setFont(m_menuFont); label.setCharacterSize(15); label.setFillColor(sf::Color::White);
            label.setString(L"Sell $" + std::to_wstring(twr->getSellValue()));
            label.setPosition(px + 5, py + 38 + 6);
            m_window.draw(label);
        }
    }
}

// ============================================================
//  作弊码系统（罪恶都市风格键盘输入）
// ============================================================

void Game::processCheatInput(sf::Uint32 unicode)
{
    // 只接受可打印 ASCII 字母，自动转小写
    if (unicode >= 'A' && unicode <= 'Z')
        unicode = unicode + ('a' - 'A');
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
            [](const std::shared_ptr<Enemy> &e) { return e->isDead(); }),
        m_enemies.end());
}

void Game::spawnBoss()
{
    const auto &waypoints = m_map.getWaypoints();
    if (waypoints.empty()) return;

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

// ============================================================
//  暂停菜单
// ============================================================

void Game::buildPauseMenu()
{
    bool zh = (LangManager::currentLangName() == "zh");

    m_pauseTitle.setFont(m_menuFont);
    m_pauseTitle.setString(zh ? L"暂停" : L"PAUSED");
    m_pauseTitle.setCharacterSize(zh ? 44 : 48);
    m_pauseTitle.setFillColor(sf::Color(255, 215, 0));
    m_pauseTitle.setStyle(sf::Text::Bold);

    TextKey keys[] = {TextKey::ContinueGame, TextKey::Settings, TextKey::Exit};
    float bw = zh ? 300.0f : 260.0f;
    m_pauseButtons.clear();
    for (int i = 0; i < 3; ++i)
    {
        PauseButton btn;
        btn.bg.setSize(sf::Vector2f(bw, 50));
        btn.bg.setFillColor(sf::Color(40, 40, 60));
        btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        btn.bg.setOutlineThickness(2);
        btn.label.setFont(m_menuFont);
        btn.label.setString(LangManager::get(keys[i]));
        btn.label.setCharacterSize(zh ? 22 : 24);
        btn.label.setFillColor(sf::Color::White);
        m_pauseButtons.push_back(btn);
    }
    // 退出按钮标红
    m_pauseButtons[2].bg.setFillColor(sf::Color(80, 30, 30));
    m_pauseButtons[2].bg.setOutlineColor(sf::Color(200, 80, 80));
}

void Game::renderPauseMenu()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    m_window.draw(overlay);

    // 对话框背景
    float dlgW = zh ? 380.0f : 360.0f;
    sf::RectangleShape dlgBg(sf::Vector2f(dlgW, 320));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - dlgW / 2, WINDOW_HEIGHT / 2.0f - 160);
    dlgBg.setFillColor(sf::Color(20, 20, 40));
    dlgBg.setOutlineColor(sf::Color(255, 215, 0));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    // 标题
    sf::FloatRect tb = m_pauseTitle.getLocalBounds();
    m_pauseTitle.setOrigin(tb.width / 2, tb.height / 2);
    m_pauseTitle.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 110);
    m_window.draw(m_pauseTitle);

    // 按钮
    for (int i = 0; i < 3; ++i)
    {
        auto &btn = m_pauseButtons[i];
        float bw = zh ? 300.0f : 260.0f;
        btn.bg.setOrigin(bw / 2, 25);
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 30 + i * 65.0f);
        m_window.draw(btn.bg);

        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 34 + i * 65.0f);
        m_window.draw(btn.label);
    }

    // 提示
    sf::Text hint;
    hint.setFont(m_menuFont);
    hint.setString(std::wstring(L"ESC: ") + LangManager::get(TextKey::ContinueGame));
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(120, 120, 140));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2, hb.height / 2);
    hint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 175);
    m_window.draw(hint);
}

void Game::processPauseEvents(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_paused = false;
            return;
        }
        return;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;
        for (int i = 0; i < 3; ++i)
        {
            bool inside = m_pauseButtons[i].bg.getGlobalBounds().contains(mx, my);
            m_pauseButtons[i].hovered = inside;
            if (i == 2)
            {
                m_pauseButtons[i].bg.setFillColor(inside ? sf::Color(180, 50, 50) : sf::Color(80, 30, 30));
                m_pauseButtons[i].bg.setOutlineColor(inside ? sf::Color(255, 100, 100) : sf::Color(200, 80, 80));
            }
            else
            {
                m_pauseButtons[i].bg.setFillColor(inside ? sf::Color(60, 60, 100) : sf::Color(40, 40, 60));
                m_pauseButtons[i].bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
            }
            m_pauseButtons[i].label.setFillColor(inside ? sf::Color(255, 215, 0) : sf::Color::White);
        }
        return;
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    for (int i = 0; i < 3; ++i)
    {
        if (m_pauseButtons[i].bg.getGlobalBounds().contains(mx, my))
        {
            switch (i)
            {
            case 0: // 继续游戏
                m_paused = false;
                break;
            case 1: // 设置
                m_paused = false;
                saveGame();
                m_stateBeforeSettings = GameState::Playing;
                m_state = GameState::Settings;
                buildSettingsUI();
                break;
            case 2: // 保存并退出
                m_paused = false;
                saveGame();
                returnToMenu();
                break;
            }
            break;
        }
    }
}
