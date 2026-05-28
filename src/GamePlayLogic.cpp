// GamePlay.cpp — Game 的游戏逻辑实现
#include "Game.h"
#include "LangManager.h"
#include <cmath>
#include <iostream>

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
