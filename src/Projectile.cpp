#include "Projectile.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f start, std::shared_ptr<Enemy> target,
                       float damage, float speed, TowerType towerType)
    : m_position(start)
    , m_target(target)
    , m_damage(damage)
    , m_speed(speed)
    , m_towerType(towerType)
    , m_hasHit(false) {

    m_shape.setRadius(4);
    m_shape.setOrigin(4, 4);
    m_shape.setPosition(start);

    switch (towerType) {
    case TowerType::Arrow:
        m_shape.setFillColor(sf::Color(150, 255, 150));
        break;
    case TowerType::Cannon:
        m_shape.setRadius(6);
        m_shape.setOrigin(6, 6);
        m_shape.setFillColor(sf::Color(255, 100, 50));
        break;
    case TowerType::Ice:
        m_shape.setFillColor(sf::Color(150, 200, 255));
        break;
    }
}

void Projectile::update(float dt) {
    if (m_hasHit) return;

    auto target = m_target.lock();
    if (!target || target->isDead() || target->hasReachedEnd()) {
        m_hasHit = true;
        return;
    }

    sf::Vector2f targetPos = target->getPosition();
    sf::Vector2f dir = targetPos - m_position;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (dist < m_speed * dt + 5.0f) {
        m_position = targetPos;
        m_hasHit = true;
    } else {
        dir /= dist;
        m_position += dir * m_speed * dt;
    }

    m_shape.setPosition(m_position);
}

void Projectile::draw(sf::RenderWindow& window) const {
    if (!m_hasHit) {
        window.draw(m_shape);
    }
}
