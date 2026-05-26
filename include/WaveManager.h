#pragma once

#include "Constants.h"
#include "Enemy.h"
#include <vector>
#include <memory>

struct WaveData {
    int enemyCount;
    float spawnInterval;  // �?
    float speed;
    float hp;
    int reward;
    sf::Color color;
};

class WaveManager {
public:
    WaveManager();

    void update(float dt, std::vector<std::shared_ptr<Enemy>>& enemies,
                const std::vector<struct Waypoint>& waypoints);

    bool isWaveComplete() const;
    bool canStartWave() const { return !m_waveActive && !m_allWavesComplete; }
    bool areAllWavesComplete() const { return m_allWavesComplete; }
    void startNextWave();

    int getCurrentWave() const { return m_currentWave; }
    int getTotalWaves() const { return static_cast<int>(m_waves.size()); }
    int getEnemiesRemaining() const { return m_remainingInWave; }

    // 自定义模式：根据参数生成波次
    void setCustomWaves(int waveCount, int enemiesPerWave, float speedMul, float hpMul);

private:
    std::vector<WaveData> m_waves;
    int m_currentWave;
    int m_spawnedInWave;
    int m_remainingInWave;
    float m_spawnTimer;
    bool m_waveActive;
    bool m_allWavesComplete;
    bool m_waveJustStarted;
};
