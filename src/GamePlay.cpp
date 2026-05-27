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
                    int cost = Tower::getStats(tt).cost;
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
    updateEnemies(dt);
    updateTowers(dt);
    towerFindTargets();
    updateProjectiles(dt);
    checkProjectileCollisions();
    m_waveManager.update(dt, m_enemies, m_map.getWaypoints());

    if (m_waveManager.areAllWavesComplete() && m_enemies.empty())
        m_state = GameState::GameWon;

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
}

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
    { text.setString(LangManager::get(TextKey::Victory)); text.setFillColor(sf::Color::Yellow); }
    else
    { text.setString(LangManager::get(TextKey::GameOver)); text.setFillColor(sf::Color::Red); }
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
    for (auto &tower : m_towers)
    {
        if (!tower->canFire()) continue;
        std::shared_ptr<Enemy> bestTarget = nullptr;
        float bestDist = tower->getRange();
        for (auto &enemy : m_enemies)
        {
            if (enemy->isDead() || enemy->hasReachedEnd()) continue;
            float dx = enemy->getPosition().x - tower->getPosition().x;
            float dy = enemy->getPosition().y - tower->getPosition().y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= tower->getRange() && (!bestTarget || dist < bestDist))
            { bestTarget = enemy; bestDist = dist; }
        }
        if (bestTarget)
        {
            m_projectiles.push_back(std::make_shared<Projectile>(
                tower->getPosition(), bestTarget, tower->getDamage(), 300.0f, tower->getType()));
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
    // 快速推进所有波次到完成
    while (m_waveManager.canStartWave())
        m_waveManager.startNextWave();
    // 如果还有波次没完成，强制标记
    while (m_waveManager.canStartWave())
        m_waveManager.startNextWave();
    m_state = GameState::GameWon;
}
