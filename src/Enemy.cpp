#include "Enemy.h"
#include "Map.h"
#include <cmath>

sf::Texture Enemy::s_textures[Enemy::MAX_VARIANTS];
int Enemy::s_variantCount = 0;
Enemy::VariantCfg Enemy::s_variants[Enemy::MAX_VARIANTS];

void Enemy::loadTextures()
{
    // 纹理路径和对应属性: {文件, hpMul, spdMul, rewardMul, weight}
    struct { const char* path; float hpMul; float spdMul; int rewardMul; int weight; } cfg[] = {
        {"textures/enemy1.png", 1.0f, 1.0f, 1, 40},  // 绿色史莱姆
        {"textures/enemy2.png", 1.6f, 1.0f, 1, 35},  // 黄色中型
        {"textures/enemy3.png", 2.5f, 0.8f, 2, 25},  // 红色重型
        // BOSS 不参与普通出怪，只在最后一波随机选一只
        {"textures/boss1.png",  5.0f, 0.5f, 3,  0},  // BOSS1（最后一波候选）
        {"textures/boss2.png",  6.0f, 0.4f, 4,  0},  // BOSS2（最后一波候选）
    };
    s_variantCount = sizeof(cfg) / sizeof(cfg[0]);

    for (int i = 0; i < s_variantCount; ++i)
    {
        s_textures[i].loadFromFile(cfg[i].path);
        s_variants[i] = {cfg[i].hpMul, cfg[i].spdMul, cfg[i].rewardMul, cfg[i].weight};
    }
}

int Enemy::getRandomVariant()
{
    int totalW = 0;
    // 只算普通怪（权重>0），BOSS权重为0不参与
    for (int i = 0; i < s_variantCount; ++i)
        totalW += s_variants[i].weight;
    int r = rand() % totalW;
    for (int i = 0; i < s_variantCount; ++i)
    {
        r -= s_variants[i].weight;
        if (r < 0) return i;
    }
    return 0;
}

int Enemy::getRandomBoss()
{
    // BOSS 从索引 3 开始到末尾随机选
    int bossCount = s_variantCount - 3;
    if (bossCount <= 0) return 0;
    return 3 + rand() % bossCount;
}

Enemy::Enemy(int waypointIndex, float speed, float hp, int reward, int variantIdx)
    : m_currentWaypoint(waypointIndex + 1), m_variant(variantIdx % s_variantCount),
      m_baseSpeed(speed * s_variants[m_variant].spdMul),
      m_hp(hp * s_variants[m_variant].hpMul),
      m_maxHp(hp * s_variants[m_variant].hpMul),
      m_reward(reward * s_variants[m_variant].rewardMul),
      m_dead(false), m_reachedEnd(false), m_slowTimer(0), m_slowFactor(1.0f)
{
    m_sprite.setTexture(s_textures[m_variant]);
    auto sz = s_textures[m_variant].getSize();
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
    auto sz = s_textures[m_variant].getSize();
    int fw = sz.x / 4;
    float scl = TILE_SIZE * 0.7f / fw;
    m_sprite.setTextureRect(sf::IntRect(m_animFrame * fw, 0, fw, sz.y));
    m_sprite.setOrigin(fw / 2.0f, sz.y / 2.0f);

    if (m_slowTimer > 0) { m_slowTimer -= dt; if (m_slowTimer <= 0) m_slowFactor = 1.0f; }
    float speed = m_baseSpeed * m_slowFactor;
    if (m_currentWaypoint < static_cast<int>(waypoints.size()))
    {
        sf::Vector2f target = waypoints[m_currentWaypoint].pos;
        sf::Vector2f current = m_sprite.getPosition();
        sf::Vector2f dir = target - current;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        // 根据水平方向翻转精灵
        float dirX = (dist < 1.0f) ? 1.0f : dir.x / dist;
        if (dirX < 0) scl = -scl;
        m_sprite.setScale(scl, TILE_SIZE * 0.7f / fw);

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
