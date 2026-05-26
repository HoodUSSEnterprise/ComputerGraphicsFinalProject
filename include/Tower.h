#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>

enum class TowerType
{
    Arrow,  // 箭塔
    Cannon, // 炮塔
    Ice     // 冰塔
};

struct TowerStats
{
    int cost;
    float range;
    float damage;
    float fireRate;
    sf::Color color;
    TowerType type;
};

class Tower
{
public:
    Tower(TowerType type, sf::Vector2f position);

    void update(float dt);
    void draw(sf::RenderWindow &window) const;

    bool canFire() const;
    void resetFireTimer();

    sf::Vector2f getPosition() const { return m_position; }
    float getRange() const { return m_stats.range; }
    float getDamage() const { return m_stats.damage; }
    TowerType getType() const { return m_stats.type; }
    int getCost() const { return m_stats.cost; }
    float getFireTimer() const { return m_fireTimer; }
    float getFireCooldown() const { return 1.0f / m_stats.fireRate; }

    // 升级系统
    int  getLevel() const { return m_level; }
    bool canUpgrade() const { return m_level < 3; }
    int  getUpgradeCost() const;
    int  getSellValue() const;
    void upgrade();

    static TowerStats getStats(TowerType type, int level = 1);

private:
    TowerStats m_stats;
    sf::Vector2f m_position;
    float m_fireTimer;
    int m_level = 1;

    sf::CircleShape m_base;
    sf::CircleShape m_rangeIndicator;
    sf::RectangleShape m_turret;

    void applyStats();
};
