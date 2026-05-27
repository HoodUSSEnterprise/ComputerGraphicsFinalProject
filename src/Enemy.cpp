#include "Enemy.h"
#include "Map.h"
#include <cmath>

sf::Texture Enemy::s_textures[3];
int Enemy::s_currentVariant = 0;

void Enemy::loadTextures()
{
    const char* paths[] = {"textures/enemy1.png","textures/enemy2.png","textures/enemy3.png"};
    for (int i = 0; i < 3; ++i)
        s_textures[i].loadFromFile(paths[i]);
}

void Enemy::setEnemyVariant(int idx)
{
    if (idx >= 0 && idx < 3) s_currentVariant = idx;
}

Enemy::Enemy(int waypointIndex, float speed, float hp, int reward)
    : m_currentWaypoint(waypointIndex + 1),
      m_baseSpeed(speed), m_hp(hp), m_maxHp(hp), m_reward(reward),
      m_dead(false), m_reachedEnd(false), m_slowTimer(0), m_slowFactor(1.0f)
{
    m_sprite.setTexture(s_textures[s_currentVariant]);
    auto sz = s_textures[s_currentVariant].getSize();
    int fw = sz.x / 4;
    m_sprite.setTextureRect(sf::IntRect(0, 0, fw, sz.y));
    m_sprite.setOrigin(fw / 2.0f, sz.y / 2.0f);
    m_sprite.setScale(TILE_SIZE * 0.7f / fw, TILE_SIZE * 0.7f / fw);

    m_hpBarBg.setSize(sf::Vector2f(TILE_SIZE / 2.0f, 4));
    m_hpBarBg.setFillColor(sf::Color(50, 50, 50));
    m_hpBar.setSize(sf::Vector2f(TILE_SIZE / 2.0f, 4));
    m_hpBar.setFillColor(sf::Color::Red);
}

void Enemy::update(float dt, const std::vector<Waypoint> &waypoints)
{
    if (m_dead || m_reachedEnd) return;
    m_animTimer += dt;
    if (m_animTimer >= 0.15f) { m_animTimer = 0; m_animFrame = (m_animFrame + 1) % 4; }
    auto sz = s_textures[s_currentVariant].getSize();
    int fw = sz.x / 4;
    m_sprite.setTextureRect(sf::IntRect(m_animFrame * fw, 0, fw, sz.y));
    m_sprite.setOrigin(fw / 2.0f, sz.y / 2.0f);
    m_sprite.setScale(TILE_SIZE * 0.7f / fw, TILE_SIZE * 0.7f / fw);

    if (m_slowTimer > 0) { m_slowTimer -= dt; if (m_slowTimer <= 0) m_slowFactor = 1.0f; }
    float speed = m_baseSpeed * m_slowFactor;
    if (m_currentWaypoint < static_cast<int>(waypoints.size()))
    {
        sf::Vector2f target = waypoints[m_currentWaypoint].pos;
        sf::Vector2f current = m_sprite.getPosition();
        sf::Vector2f dir = target - current;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist < speed * dt + 2.0f) { m_sprite.setPosition(target); m_currentWaypoint++; }
        else { dir /= dist; m_sprite.move(dir * speed * dt); }
    }
    else { m_reachedEnd = true; }

    sf::Vector2f pos = m_sprite.getPosition();
    m_hpBarBg.setPosition(pos.x - TILE_SIZE / 4.0f, pos.y - TILE_SIZE / 2.0f);
    float hpRatio = m_hp / m_maxHp;
    m_hpBar.setSize(sf::Vector2f(TILE_SIZE / 2.0f * hpRatio, 4));
    m_hpBar.setPosition(pos.x - TILE_SIZE / 4.0f, pos.y - TILE_SIZE / 2.0f);
    if (hpRatio > 0.5f) m_hpBar.setFillColor(sf::Color::Green);
    else if (hpRatio > 0.25f) m_hpBar.setFillColor(sf::Color::Yellow);
    else m_hpBar.setFillColor(sf::Color::Red);
}

void Enemy::draw(sf::RenderWindow &window) const
{
    if (m_dead || m_reachedEnd) return;
    window.draw(m_sprite);
    window.draw(m_hpBarBg);
    window.draw(m_hpBar);
}

void Enemy::takeDamage(float damage) { m_hp -= damage; if (m_hp <= 0) { m_hp = 0; m_dead = true; } }
void Enemy::applySlow(float factor, float duration) { m_slowFactor = factor; m_slowTimer = duration; }
