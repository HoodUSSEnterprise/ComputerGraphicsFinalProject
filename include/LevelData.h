#pragma once

#include "Constants.h"
#include "LangManager.h"
#include <string>
#include <vector>

// 群系
enum class Biome { Grassland, Desert, Hell, Community, COUNT };

// 单个关卡配置
struct LevelConfig
{
    std::string id;
    std::string mapFile;
    Biome biome;
    TextKey nameKey;
    TextKey descKey;
    int startGold;
    int startLives;
    int waveCount;
    int baseEnemies;
    float speedMul;
    float hpMul;
};

// 自定义模式参数
struct CustomParams
{
    int waves = 5;
    int enemiesPerWave = 10;
    int startGold = 200;
    int startLives = 20;
    float speedMul = 1.0f;
    float hpMul = 1.0f;
    std::string mapFile = "assets/maps/grassland/1-1.txt";
};

// 所有战役关卡
inline std::vector<LevelConfig> getCampaignLevels()
{
    return {
        // ===== 翠绿平原 (Grassland) =====
        {"1-1", "assets/maps/grassland/1-1.txt", Biome::Grassland, TextKey::L11_Name, TextKey::L11_Desc, 250, 25, 5,  8,  0.9f, 0.8f},
        {"1-2", "assets/maps/grassland/1-2.txt", Biome::Grassland, TextKey::L12_Name, TextKey::L12_Desc, 220, 22, 6,  9,  1.0f, 0.9f},
        {"1-3", "assets/maps/grassland/1-3.txt", Biome::Grassland, TextKey::L13_Name, TextKey::L13_Desc, 200, 20, 7,  10, 1.1f, 1.0f},

        // ===== 沙漠风暴 (Desert) =====
        {"2-1", "assets/maps/desert/2-1.txt", Biome::Desert, TextKey::L21_Name, TextKey::L21_Desc, 200, 20, 7,  10, 1.1f, 1.2f},
        {"2-2", "assets/maps/desert/2-2.txt", Biome::Desert, TextKey::L22_Name, TextKey::L22_Desc, 180, 18, 8,  12, 1.2f, 1.4f},
        {"2-3", "assets/maps/desert/2-3.txt", Biome::Desert, TextKey::L23_Name, TextKey::L23_Desc, 160, 16, 9,  14, 1.3f, 1.6f},

        // ===== 地狱深渊 (Hell) =====
        {"3-1", "assets/maps/hell/3-1.txt", Biome::Hell, TextKey::L31_Name, TextKey::L31_Desc, 180, 18, 8,  12, 1.3f, 1.5f},
        {"3-2", "assets/maps/hell/3-2.txt", Biome::Hell, TextKey::L32_Name, TextKey::L32_Desc, 160, 16, 9,  14, 1.5f, 1.8f},
        {"3-3", "assets/maps/hell/3-3.txt", Biome::Hell, TextKey::L33_Name, TextKey::L33_Desc, 140, 14, 10, 16, 1.7f, 2.0f},

        // ===== 社区关卡 (Community) =====
        {"c1", "assets/maps/community/c1.txt", Biome::Community, TextKey::C1_Name, TextKey::C1_Desc, 300, 30, 8,  10, 1.0f, 0.9f},
        {"c2", "assets/maps/community/c2.txt", Biome::Community, TextKey::C2_Name, TextKey::C2_Desc, 250, 25, 10, 12, 1.2f, 1.1f},
    };
}

