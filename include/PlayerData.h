#pragma once

#include <string>
#include <vector>
#include <fstream>

// 商店升级项目
enum class ShopItem
{
    StartGoldBonus, // 每级 +50 起始金币
    TowerDiscount,  // 每级 -10% 建塔费用
    DamageBoost,    // 每级 +15% 塔伤害
    RangeBoost,     // 每级 +8% 塔射程
    LivesBonus,     // 每级 +3 起始生命
    FireRateBoost,  // 每级 +10% 攻速

    COUNT
};

constexpr int SHOP_ITEM_COUNT = static_cast<int>(ShopItem::COUNT);
constexpr int SHOP_MAX_LEVEL = 5; // 每个项目最多5级

struct ShopItemDef
{
    const char *nameKey; // 语言文件key
    int baseCost;        // 第1级价格
    int costPerLevel;    // 每级递增价格
};

// 商店项目定义（名称在语言文件中）
inline ShopItemDef getShopItemDef(ShopItem item)
{
    switch (item)
    {
    case ShopItem::StartGoldBonus:
        return {"Shop_StartGold", 80, 40};
    case ShopItem::TowerDiscount:
        return {"Shop_Discount", 100, 50};
    case ShopItem::DamageBoost:
        return {"Shop_Damage", 120, 60};
    case ShopItem::RangeBoost:
        return {"Shop_Range", 100, 50};
    case ShopItem::LivesBonus:
        return {"Shop_Lives", 90, 45};
    case ShopItem::FireRateBoost:
        return {"Shop_FireRate", 110, 55};
    default:
        return {"", 0, 0};
    }
}

// 角色存档数据
struct PlayerData
{
    std::string name;
    int totalGold = 0;                    // 跨关卡累计金币
    int unlockedLevels = 1;               // 已解锁关卡数（至少为1）
    int shopLevels[SHOP_ITEM_COUNT] = {}; // 商店各项目等级

    // ---- 从商店升级获取加成 ----
    int getStartGoldBonus() const { return shopLevels[static_cast<int>(ShopItem::StartGoldBonus)] * 50; }
    float getTowerDiscount() const { return shopLevels[static_cast<int>(ShopItem::TowerDiscount)] * 0.10f; }
    float getDamageBoost() const { return shopLevels[static_cast<int>(ShopItem::DamageBoost)] * 0.15f; }
    float getRangeBoost() const { return shopLevels[static_cast<int>(ShopItem::RangeBoost)] * 0.08f; }
    int getLivesBonus() const { return shopLevels[static_cast<int>(ShopItem::LivesBonus)] * 3; }
    float getFireRateBoost() const { return shopLevels[static_cast<int>(ShopItem::FireRateBoost)] * 0.10f; }

    int getUpgradeCost(ShopItem item) const
    {
        auto def = getShopItemDef(item);
        int lv = shopLevels[static_cast<int>(item)];
        if (lv >= SHOP_MAX_LEVEL)
            return 0;
        return def.baseCost + lv * def.costPerLevel;
    }

    bool canUpgrade(ShopItem item) const
    {
        return shopLevels[static_cast<int>(item)] < SHOP_MAX_LEVEL;
    }

    void upgrade(ShopItem item)
    {
        int idx = static_cast<int>(item);
        if (shopLevels[idx] < SHOP_MAX_LEVEL)
        {
            totalGold -= getUpgradeCost(item);
            shopLevels[idx]++;
        }
    }

    // ---- 存档/读档 ----
    bool save(const std::string &path) const;
    bool load(const std::string &path);

    static std::vector<PlayerData> listCharacters(const std::string &dir);
    static std::string makeSavePath(const std::string &name);
};
