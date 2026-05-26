#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>

enum class TowerType
{
    Arrow,  // 箭塔: 快速、低伤害
    Cannon, // 炮塔: 慢速、范围伤�?
    Ice     // 冰塔: 减�?
};

struct TowerStats
{
    int cost;
    float range;
    float damage;
    float fireRate; // 每秒攻击次数
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

    static TowerStats getStats(TowerType type);

private:
    TowerStats m_stats;
    sf::Vector2f m_position;
    float m_fireTimer;

    // 图形
    sf::CircleShape m_base;
    sf::CircleShape m_rangeIndicator;
    sf::RectangleShape m_turret;
};
