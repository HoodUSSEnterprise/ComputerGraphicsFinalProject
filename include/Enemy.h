#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>

class Enemy
{
public:
    Enemy(int waypointIndex, float speed, float hp, int reward, int variantIdx = 0);

    void update(float dt, const std::vector<struct Waypoint> &waypoints);
    void draw(sf::RenderWindow &window) const;

    void takeDamage(float damage);
    bool isDead() const { return m_dead; }
    bool hasReachedEnd() const { return m_reachedEnd; }
    bool isSlowed() const { return m_slowTimer > 0; }
    void applySlow(float factor, float duration);

    float getHP() const { return m_hp; }
    float getMaxHP() const { return m_maxHp; }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    void setPosition(sf::Vector2f pos) { m_sprite.setPosition(pos); }
    float getSpeed() const { return m_baseSpeed; }
    int getReward() const { return m_reward; }

    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }

    static void loadTextures();
    static int getRandomVariant();
    static int getRandomBoss();     // 随机选一只 BOSS

private:
    // 纹理和变体配置
    static const int MAX_VARIANTS = 8;
    static sf::Texture s_textures[MAX_VARIANTS];
    static int s_variantCount;
    struct VariantCfg { float hpMul; float spdMul; int rewardMul; int weight; };
    static VariantCfg s_variants[MAX_VARIANTS];
    friend void initVariantConfig();

    mutable sf::Sprite m_sprite;
    sf::RectangleShape m_hpBar;
    sf::RectangleShape m_hpBarBg;

    float m_animTimer = 0;
    int m_animFrame = 0;
    int m_variant = 0;

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
