#pragma once

#include "Constants.h"
#include "LangManager.h"
#include <string>
#include <vector>

// 单个关卡配置
struct LevelConfig {
    std::string id;          // 内部标识
    TextKey nameKey;         // 名称文本键
    TextKey descKey;         // 描述文本键
    int startGold;           // 初始金币
    int startLives;          // 初始生命
    int waveCount;           // 波次数
    int baseEnemies;         // 基础每波敌人数
    float speedMul;          // 速度倍率
    float hpMul;             // 生命倍率
};

// 自定义模式参数
struct CustomParams {
    int waves = 5;
    int enemiesPerWave = 10;
    int startGold = 200;
    int startLives = 20;
    float speedMul = 1.0f;
    float hpMul = 1.0f;
};

// 三关战役定义
inline std::vector<LevelConfig> getCampaignLevels() {
    return {
        {
            "level1",
            TextKey::Level1_Name,
            TextKey::Level1_Desc,
            250,   // startGold
            25,    // startLives
            5,     // waveCount
            8,     // baseEnemies
            0.9f,  // speedMul
            0.8f,  // hpMul
        },
        {
            "level2",
            TextKey::Level2_Name,
            TextKey::Level2_Desc,
            200,   // startGold
            20,    // startLives
            7,     // waveCount
            10,    // baseEnemies
            1.1f,  // speedMul
            1.2f,  // hpMul
        },
        {
            "level3",
            TextKey::Level3_Name,
            TextKey::Level3_Desc,
            150,   // startGold
                    15,    // startLives
            10,    // waveCount
            12,    // baseEnemies
            1.4f,  // speedMul
            1.6f,  // hpMul
        },
    };
}
