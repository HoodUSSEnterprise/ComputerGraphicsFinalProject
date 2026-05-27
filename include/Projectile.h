#pragma once

#include "Constants.h"
#include "Enemy.h"
#include "Tower.h"
#include <SFML/Graphics.hpp>
#include <memory>

class Projectile
{
public:
    Projectile(sf::Vector2f start, std::shared_ptr<Enemy> target,
               float damage, float speed, TowerType towerType);

    // 攻击固定位置（宝藏等）
    Projectile(sf::Vector2f start, sf::Vector2f targetPos,
               float damage, float speed, TowerType towerType);

    void update(float dt);
    void draw(sf::RenderWindow &window) const;

    bool hasHit() const { return m_hasHit; }
    std::shared_ptr<Enemy> getTarget() const { return m_target.lock(); }
    float getDamage() const { return m_damage; }
    TowerType getTowerType() const { return m_towerType; }
    bool isTreasureShot() const { return m_isTreasureShot; }

private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    std::weak_ptr<Enemy> m_target;
    sf::Vector2f m_targetPos;
    bool m_isTreasureShot = false;
    float m_damage;
    float m_speed;
    TowerType m_towerType;
    bool m_hasHit;

    sf::CircleShape m_shape;
};
