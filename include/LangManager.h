#pragma once

#include <string>
#include <unordered_map>

enum class TextKey
{
    Title,
    Subtitle,
    NewGame,
    LoadGame,
    Settings,
    Exit,
    MenuHint,
    Language_Label,
    Gold,
    Lives,
    Wave,
    StartWave,
    Instruction,
    Tower_Arrow,
    Tower_Cannon,
    Tower_Ice,
    Msg_CannotPlace,
    Msg_NoGold,
    Msg_AlreadyHere,
    Msg_Placed,
    Msg_Sold,
    Msg_WaveStarted,
    Victory,
    GameOver,
    PressR,
    EscToMenu,
    Back,
    Volume,
    BGM_On,
    BGM_Off,
    CustomMode,
    CustomWaves,
    CustomEnemies,
    CustomGold,
    CustomLives,
    CustomSpeed,
    CustomHP,
    CustomStart,
    Campaign,
    Level1_Name,
    Level1_Desc,
    Level2_Name,
    Level2_Desc,
    Level3_Name,
    Level3_Desc,
    Biome_Grassland,
    Biome_Desert,
    Biome_Hell,
    Biome_Community,
    L11_Name,
    L11_Desc,
    L12_Name,
    L12_Desc,
    L13_Name,
    L13_Desc,
    L21_Name,
    L21_Desc,
    L22_Name,
    L22_Desc,
    L23_Name,
    L23_Desc,
    L31_Name,
    L31_Desc,
    L32_Name,
    L32_Desc,
    L33_Name,
    L33_Desc,
    C1_Name,
    C1_Desc,
    C2_Name,
    C2_Desc,

    // 角色系统
    CharSelect_Title,
    CharSelect_New,
    CharSelect_Confirm,
    CharSelect_EnterName,
    CharSelect_Empty,
    CharSelect_Delete,
    CharSelect_Gold,
    CharSelect_Progress,

    // 主菜单（新）
    ContinueGame,
    NewGame_WarnTitle,
    NewGame_WarnMsg,
    NewGame_ConfirmYes,
    NewGame_ConfirmNo,

    // 商店
    Shop_Title,
    Shop_Gold,
    Shop_Buy,
    Shop_Max,
    Shop_StartGold,
    Shop_Discount,
    Shop_Damage,
    Shop_Range,
    Shop_Lives,
    Shop_FireRate,
    Shop_Desc_StartGold,
    Shop_Desc_Discount,
    Shop_Desc_Damage,
    Shop_Desc_Range,
    Shop_Desc_Lives,
    Shop_Desc_FireRate,

    // 关卡锁定
    Level_Locked,
    Level_Complete,

    // 通关界面
    NextLevel,
    BackToMenu,

    COUNT
};

class LangManager
{
public:
    static bool loadLanguage(const std::string &jsonPath);
    static const std::wstring &get(TextKey key);
    static const std::string &currentLangName() { return s_currentLang; }
    static const std::string &getFontPath() { return s_fontPath; }

    static TextKey keyFromString(const std::string &name);

private:
    static bool parseFlatJson(const std::string &content,
                              std::unordered_map<std::string, std::string> &out);
    // UTF-8 → wstring 转换
    static std::wstring utf8ToWide(const std::string &utf8);

    static std::string s_currentLang;
    static std::string s_fontPath;
    static std::wstring s_texts[static_cast<int>(TextKey::COUNT)];
    static bool s_loaded;
};
