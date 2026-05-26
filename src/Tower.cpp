#include "Tower.h"
#include <cmath>

TowerStats Tower::getStats(TowerType type)
{
    switch (type)
    {
    case TowerType::Arrow:
        return {50, 150.0f, 15.0f, 2.0f, sf::Color(100, 200, 100), type};
    case TowerType::Cannon:
        return {100, 120.0f, 50.0f, 0.8f, sf::Color(200, 100, 100), type};
    case TowerType::Ice:
        return {75, 130.0f, 8.0f, 1.5f, sf::Color(100, 150, 255), type};
    }
    return {0, 0, 0, 0, sf::Color::White, type};
}

Tower::Tower(TowerType type, sf::Vector2f position)
    : m_stats(getStats(type)), m_position(position), m_fireTimer(0)
{

    // 底座
    m_base.setRadius(TILE_SIZE / 3.0f);
    m_base.setOrigin(TILE_SIZE / 3.0f, TILE_SIZE / 3.0f);
    m_base.setPosition(position);
    m_base.setFillColor(m_stats.color);
    m_base.setOutlineColor(sf::Color::White);
    m_base.setOutlineThickness(2);

    // 射程指示器（半透明�?
    m_rangeIndicator.setRadius(m_stats.range);
    m_rangeIndicator.setOrigin(m_stats.range, m_stats.range);
    m_rangeIndicator.setPosition(position);
    m_rangeIndicator.setFillColor(sf::Color(255, 255, 255, 30));
    m_rangeIndicator.setOutlineColor(sf::Color(255, 255, 255, 60));
    m_rangeIndicator.setOutlineThickness(1);

    // 炮管
    m_turret.setSize(sf::Vector2f(6, TILE_SIZE / 3.0f));
    m_turret.setOrigin(3, TILE_SIZE / 3.0f);
    m_turret.setPosition(position);
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
