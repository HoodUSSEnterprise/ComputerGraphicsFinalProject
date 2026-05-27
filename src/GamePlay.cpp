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
        else if (m_map.isTreasure(grid.x, grid.y))
        {
            // 标记宝藏为目标，塔会在 towerFindTargets 中攻击
            m_treasureTarget = grid;
            hidePopup();
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

    m_map.updateTreasureTimers(dt);
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

    // 如果宝藏被标记，让范围内塔向其开火
    bool treasureAlive = m_map.isTreasure(m_treasureTarget.x, m_treasureTarget.y);

    for (auto &tower : m_towers)
    {
        if (!tower->canFire()) continue;

        // 优先攻击宝藏（如果被标记且在范围内）
        if (treasureAlive)
        {
            sf::Vector2f tpos = m_map.getTreasureWorldPos(m_treasureTarget.x, m_treasureTarget.y);
            float dx = tower->getPosition().x - tpos.x;
            float dy = tower->getPosition().y - tpos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float range = tower->getRange() * rangeMul;
            if (dist <= range)
            {
                if (tower->getType() != TowerType::Ice)
                {
                    float adx = tpos.x - tower->getPosition().x;
                    float ady = tpos.y - tower->getPosition().y;
                    tower->setTargetAngle(std::atan2(ady, adx) * 180.0f / 3.14159265f + 90.0f);
                }
                m_projectiles.push_back(std::make_shared<Projectile>(
                    tower->getPosition(), tpos + sf::Vector2f(0, TILE_SIZE/2),
                    tower->getDamage() * dmgMul, 400.0f, tower->getType()));
                tower->resetFireTimer();
                if (frMul > 1.0f) tower->applyFireRateBoost(frMul);
                continue;
            }
        }

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
            // 计算炮口朝向角度（Ice除外）
            if (tower->getType() != TowerType::Ice)
            {
                float dx = bestTarget->getPosition().x - tower->getPosition().x;
                float dy = bestTarget->getPosition().y - tower->getPosition().y;
                float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f + 90.0f;
                tower->setTargetAngle(angle);
            }
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
            if (proj->isTreasureShot())
            {
                // 宝藏弹丸命中
                int gx = m_treasureTarget.x, gy = m_treasureTarget.y;
                if (m_map.isTreasure(gx, gy))
                {
                    int gold = m_map.damageTreasure(gx, gy);
                    if (gold > 0)
                    {
                        m_gold += gold;
                        m_ui.showMessage(L"+ " + std::to_wstring(gold) + L" Gold!");
                        m_treasureTarget = {-1, -1};
                    }
                }
            }
            else
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