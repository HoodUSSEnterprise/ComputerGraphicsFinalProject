#include "Tower.h"
#include <cmath>

TowerStats Tower::getStats(TowerType type, int level)
{
    float mul = 1.0f + (level - 1) * 0.6f;
    switch (type)
    {
    case TowerType::Arrow:
        return {50 + 30 * (level - 1), 150.0f + 10 * (level - 1), 15.0f * mul, 2.0f + 0.2f * (level - 1), sf::Color(100, 200, 100), type};
    case TowerType::Cannon:
        return {100 + 50 * (level - 1), 120.0f + 8 * (level - 1), 50.0f * mul, 0.8f + 0.1f * (level - 1), sf::Color(200, 100, 100), type};
    case TowerType::Ice:
        return {75 + 40 * (level - 1), 130.0f + 10 * (level - 1), 8.0f * mul, 1.5f + 0.2f * (level - 1), sf::Color(100, 150, 255), type};
    }
    return {0, 0, 0, 0, sf::Color::White, type};
}

int Tower::getUpgradeCost() const
{
    if (m_level >= 3) return 0;
    return getStats(m_stats.type, m_level + 1).cost / 2;
}

int Tower::getSellValue() const
{
    int total = 0;
    for (int lv = 1; lv <= m_level; ++lv)
        total += getStats(m_stats.type, lv).cost;
    return total * 3 / 5;
}

void Tower::upgrade()
{
    if (m_level >= 3) return;
    ++m_level;
    m_stats = getStats(m_stats.type, m_level);
    applyStats();
}

Tower::Tower(TowerType type, sf::Vector2f position)
    : m_stats(getStats(type)), m_position(position), m_fireTimer(0)
{
    applyStats();
    m_rangeIndicator.setPosition(position);
    m_rangeIndicator.setRadius(m_stats.range);
    m_rangeIndicator.setOrigin(m_stats.range, m_stats.range);
    m_rangeIndicator.setFillColor(sf::Color(255, 255, 255, 30));
    m_rangeIndicator.setOutlineColor(sf::Color(255, 255, 255, 60));
    m_rangeIndicator.setOutlineThickness(1);
}

void Tower::applyStats()
{
    m_base.setRadius(TILE_SIZE / 3.0f);
    m_base.setOrigin(TILE_SIZE / 3.0f, TILE_SIZE / 3.0f);
    m_base.setPosition(m_position);
    m_base.setFillColor(m_stats.color);
    m_base.setOutlineColor(sf::Color::White);
    m_base.setOutlineThickness(2);

    m_turret.setSize(sf::Vector2f(6, TILE_SIZE / 3.0f));
    m_turret.setOrigin(3, TILE_SIZE / 3.0f);
    m_turret.setPosition(m_position);
    m_turret.setFillColor(sf::Color(50, 50, 50));
}

void Tower::update(float dt)
{
    if (m_fireTimer > 0)
    {
        m_fireTimer -= dt;
    }
}

void Tower::draw(sf::RenderWindow &window) const
{
    window.draw(m_rangeIndicator);
    window.draw(m_base);
    window.draw(m_turret);
}

bool Tower::canFire() const
{
    return m_fireTimer <= 0;
}

void Tower::resetFireTimer()
{
    m_fireTimer = 1.0f / m_stats.fireRate;
}
