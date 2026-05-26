#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>

class Enemy
{
public:
    Enemy(int waypointIndex, float speed, float hp, int reward);

    void update(float dt, const std::vector<struct Waypoint> &waypoints);
    void draw(sf::RenderWindow &window) const;

    void takeDamage(float damage);
    bool isDead() const { return m_dead; }
    bool hasReachedEnd() const { return m_reachedEnd; }
    bool isSlowed() const { return m_slowTimer > 0; }
    void applySlow(float factor, float duration);

    float getHP() const { return m_hp; }
    float getMaxHP() const { return m_maxHp; }
    sf::Vector2f getPosition() const { return m_shape.getPosition(); }
    void setPosition(sf::Vector2f pos) { m_shape.setPosition(pos); }
    float getSpeed() const { return m_baseSpeed; }
    int getReward() const { return m_reward; }

    // 用于范围检�?
    sf::FloatRect getBounds() const { return m_shape.getGlobalBounds(); }

private:
    sf::CircleShape m_shape;
    sf::RectangleShape m_hpBar;
    sf::RectangleShape m_hpBarBg;

    int m_currentWaypoint;
    float m_baseSpeed;
    float m_hp;
    float m_maxHp;
    int m_reward;
    bool m_dead;
    bool m_reachedEnd;

    float m_slowTimer;
    float m_slowFactor;
};
