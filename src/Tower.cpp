#include "Tower.h"
#include <cmath>
#include <iostream>

sf::Texture Tower::s_textures[3][3];
bool Tower::s_texturesLoaded = false;

void Tower::loadTextures()
{
    const char *names[] = {"Arrow", "Cannon", "Ice"};
    for (int t = 0; t < 3; ++t)
    {
        for (int lv = 1; lv <= 3; ++lv)
        {
            std::string path = "textures/" + std::string(names[t]) + "_level" + std::to_string(lv) + ".png";
            if (s_textures[t][lv - 1].loadFromFile(path))
                std::cout << "[Tower] " << path << " loaded" << std::endl;
            else
                std::cerr << "[Tower] " << path << " FAILED" << std::endl;
        }
    }
    s_texturesLoaded = true;
}

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
    int t = static_cast<int>(m_stats.type);
    int lv = m_level - 1;
    if (s_texturesLoaded && s_textures[t][lv].getSize().x > 0)
    {
        m_sprite.setTexture(s_textures[t][lv]);
        auto sz = s_textures[t][lv].getSize();
        float scale = TILE_SIZE * 0.85f / std::max(sz.x, sz.y);
        m_sprite.setOrigin(sz.x / 2.0f, sz.y / 2.0f);
        m_sprite.setScale(scale, scale);
        m_sprite.setPosition(m_position);
    }
}

void Tower::update(float dt)
{
    if (m_fireTimer > 0)
        m_fireTimer -= dt;
}

void Tower::draw(sf::RenderWindow &window) const
{
    window.draw(m_rangeIndicator);
    window.draw(m_sprite);
}

bool Tower::canFire() const
{
    return m_fireTimer <= 0;
}

void Tower::resetFireTimer()
{
    m_fireTimer = 1.0f / m_stats.fireRate;
}

void Tower::applyFireRateBoost(float mult)
{
    if (mult > 1.0f && m_fireTimer > 0)
        m_fireTimer /= mult;
}
