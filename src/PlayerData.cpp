#include "PlayerData.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

std::string PlayerData::makeSavePath(const std::string &name)
{
    std::string safe = name;
    for (auto &c : safe)
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            c = '_';
    return "saves/" + safe + ".sav";
}

bool PlayerData::save(const std::string &path) const
{
    fs::create_directories("saves");
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    // 写入名字长度+名字
    size_t nameLen = name.size();
    f.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
    f.write(name.data(), nameLen);

    f.write(reinterpret_cast<const char*>(&totalGold), sizeof(totalGold));
    f.write(reinterpret_cast<const char*>(&unlockedLevels), sizeof(unlockedLevels));

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
        f.write(reinterpret_cast<const char*>(&shopLevels[i]), sizeof(shopLevels[i]));

    return f.good();
}

bool PlayerData::load(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    size_t nameLen = 0;
    f.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
    name.resize(nameLen);
    f.read(&name[0], nameLen);

    f.read(reinterpret_cast<char*>(&totalGold), sizeof(totalGold));
    f.read(reinterpret_cast<char*>(&unlockedLevels), sizeof(unlockedLevels));

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
        f.read(reinterpret_cast<char*>(&shopLevels[i]), sizeof(shopLevels[i]));

    return f.good();
}

std::vector<PlayerData> PlayerData::listCharacters(const std::string &dir)
{
    std::vector<PlayerData> result;
    std::error_code ec;
    for (const auto &entry : fs::directory_iterator(dir, ec))
    {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext != ".sav") continue;

        PlayerData pd;
        if (pd.load(entry.path().string()))
        {
            // 仅添加能成功加载的
            result.push_back(pd);
        }
    }
    return result;
}
