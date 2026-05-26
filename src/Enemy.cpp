#include "Enemy.h"
#include "Map.h"
#include <cmath>

Enemy::Enemy(int waypointIndex, float speed, float hp, int reward)
    : m_currentWaypoint(waypointIndex + 1) // +1 因为出生在第一个路径点
      ,
      m_baseSpeed(speed), m_hp(hp), m_maxHp(hp), m_reward(reward), m_dead(false), m_reachedEnd(false), m_slowTimer(0), m_slowFactor(1.0f)
{

    m_shape.setRadius(TILE_SIZE / 4.0f);
    m_shape.setOrigin(TILE_SIZE / 4.0f, TILE_SIZE / 4.0f);
    m_shape.setFillColor(sf::Color(220, 50, 50));
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(1);

    // 血条背�?
    m_hpBarBg.setSize(sf::Vector2f(TILE_SIZE / 2.0f, 4));
    m_hpBarBg.setFillColor(sf::Color(50, 50, 50));

    // 血�?
    m_hpBar.setSize(sf::Vector2f(TILE_SIZE / 2.0f, 4));
    m_hpBar.setFillColor(sf::Color::Red);
}

void Enemy::update(float dt, const std::vector<Waypoint> &waypoints)
{
    if (m_dead || m_reachedEnd)
        return;

    // 减速计时器
    if (m_slowTimer > 0)
    {
        m_slowTimer -= dt;
        if (m_slowTimer <= 0)
        {
            m_slowFactor = 1.0f;
        }
    }

    float speed = m_baseSpeed * m_slowFactor;

    // 向当前路径点移动
    if (m_currentWaypoint < static_cast<int>(waypoints.size()))
    {
        sf::Vector2f target = waypoints[m_currentWaypoint].pos;
        sf::Vector2f current = m_shape.getPosition();
        sf::Vector2f dir = target - current;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist < speed * dt + 2.0f)
        {
            // 到达路径�?
            m_shape.setPosition(target);
            m_currentWaypoint++;
        }
        else
        {
            dir /= dist;
            m_shape.move(dir * speed * dt);
        }
    }
    else
    {
        // 到达终点
        m_reachedEnd = true;
    }

    // 更新血条位�?
    sf::Vector2f pos = m_shape.getPosition();
    m_hpBarBg.setPosition(pos.x - TILE_SIZE / 4.0f, pos.y - TILE_SIZE / 2.0f);
    float hpRatio = m_hp / m_maxHp;
    m_hpBar.setSize(sf::Vector2f(TILE_SIZE / 2.0f * hpRatio, 4));
    m_hpBar.setPosition(pos.x - TILE_SIZE / 4.0f, pos.y - TILE_SIZE / 2.0f);

    // 血量低时变�?
    if (hpRatio > 0.5f)
        m_hpBar.setFillColor(sf::Color::Green);
    else if (hpRatio > 0.25f)
        m_hpBar.setFillColor(sf::Color::Yellow);
    else
        m_hpBar.setFillColor(sf::Color::Red);
}

void Enemy::draw(sf::RenderWindow &window) const
{
    if (m_dead || m_reachedEnd)
        return;
    window.draw(m_shape);
    window.draw(m_hpBarBg);
    window.draw(m_hpBar);
}

void Enemy::takeDamage(float damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_dead = true;
    }
}

void Enemy::applySlow(float factor, float duration)
{
    m_slowFactor = factor;
    m_slowTimer = duration;
}
