#include "WaveManager.h"
#include "Map.h"
#include <cmath>

WaveManager::WaveManager()
    : m_currentWave(0), m_spawnedInWave(0), m_remainingInWave(0), m_spawnTimer(0), m_waveActive(false), m_allWavesComplete(false), m_waveJustStarted(false)
{

    // 定义波次: 数量, 间隔, 速度, 血�? 奖励, 颜色
    m_waves = {
        {8, 1.0f, 80.0f, 50.0f, 10, sf::Color(220, 50, 50)},
        {12, 0.9f, 85.0f, 70.0f, 12, sf::Color(220, 100, 50)},
        {15, 0.8f, 90.0f, 100.0f, 15, sf::Color(200, 50, 150)},
        {18, 0.7f, 95.0f, 140.0f, 18, sf::Color(50, 150, 200)},
        {20, 0.6f, 100.0f, 200.0f, 20, sf::Color(200, 200, 50)},
    };
}

void WaveManager::update(float dt, std::vector<std::shared_ptr<Enemy>> &enemies,
                         const std::vector<Waypoint> &waypoints)
{

    if (!m_waveActive || m_allWavesComplete)
        return;

    if (m_spawnedInWave >= static_cast<int>(m_waves[m_currentWave].enemyCount))
    {
        // 本波已全部生�?
        m_waveActive = false;
        return;
    }

    m_spawnTimer -= dt;
    if (m_spawnTimer <= 0)
    {
        const auto &wd = m_waves[m_currentWave];
        bool isLastWave = (m_currentWave == static_cast<int>(m_waves.size()));

        // 最后一波首只怪 → 随机 BOSS，仅此一只
        int variant;
        if (isLastWave && !m_boss2Spawned && m_spawnedInWave == 0) {
            variant = Enemy::getRandomBoss();
            m_boss2Spawned = true;
        } else {
            variant = Enemy::getRandomVariant();
        }

        auto enemy = std::make_shared<Enemy>(
            0, wd.speed, wd.hp, wd.reward, variant);

        // 设置出生位置为起点
        if (!waypoints.empty())
        {
            enemy->setPosition(waypoints[0].pos);
        }

        // 设置颜色
        // 通过重新设置位置来修正（Enemy构造后需要手动调整）

        enemies.push_back(enemy);
        m_spawnedInWave++;
        m_remainingInWave--;

        m_spawnTimer = m_waves[m_currentWave].spawnInterval;
    }
}

bool WaveManager::isWaveComplete() const
{
    if (m_allWavesComplete)
        return true;
    if (!m_waveActive)
    {
        if (m_currentWave >= static_cast<int>(m_waves.size()))
            return true;
        return m_spawnedInWave >= static_cast<int>(m_waves[m_currentWave].enemyCount);
    }
    return false;
}

void WaveManager::startNextWave()
{
    if (m_currentWave >= static_cast<int>(m_waves.size()))
    {
        m_allWavesComplete = true;
        return;
    }

    const auto &wd = m_waves[m_currentWave];
    m_spawnedInWave = 0;
    m_remainingInWave = wd.enemyCount;
    m_spawnTimer = 0.5f; // 第一只快速出�?
    m_waveActive = true;
    m_waveJustStarted = true;
    m_currentWave++; // 推进到下一�?
}

void WaveManager::setCustomWaves(int waveCount, int enemiesPerWave, float speedMul, float hpMul)
{
    m_waves.clear();
    m_currentWave = 0;
    m_spawnedInWave = 0;
    m_remainingInWave = 0;
    m_spawnTimer = 0;
    m_waveActive = false;
    m_allWavesComplete = false;
    m_waveJustStarted = false;

    sf::Color colors[] = {
        sf::Color(220, 50, 50),
        sf::Color(220, 100, 50),
        sf::Color(200, 50, 150),
        sf::Color(50, 150, 200),
        sf::Color(200, 200, 50),
        sf::Color(50, 200, 100),
        sf::Color(200, 50, 200),
        sf::Color(100, 200, 200),
    };

    for (int i = 0; i < waveCount; ++i)
    {
        WaveData wd;
        wd.enemyCount = enemiesPerWave + i * 2;
        wd.spawnInterval = std::max(0.3f, 1.0f - i * 0.05f);
        wd.speed = (80.0f + i * 5.0f) * speedMul;
        wd.hp = (50.0f + i * 20.0f) * hpMul;
        wd.reward = 10 + i * 3;
        wd.color = colors[i % 8];
        m_waves.push_back(wd);
    }
}
