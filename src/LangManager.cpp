#include "LangManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::string LangManager::s_currentLang = "English";
std::string LangManager::s_fontPath = "fonts/arial.ttf";
std::wstring LangManager::s_texts[static_cast<int>(TextKey::COUNT)];
bool LangManager::s_loaded = false;

// ============================================================
//  Key -> TextKey 映射表
// ============================================================
struct KeyEntry
{
    const char *name;
    TextKey key;
};

static const KeyEntry KEY_MAP[] = {
    {"Title", TextKey::Title},
    {"Subtitle", TextKey::Subtitle},
    {"NewGame", TextKey::NewGame},
    {"LoadGame", TextKey::LoadGame},
    {"Settings", TextKey::Settings},
    {"Exit", TextKey::Exit},
    {"MenuHint", TextKey::MenuHint},
    {"Language_Label", TextKey::Language_Label},
    {"Gold", TextKey::Gold},
    {"Lives", TextKey::Lives},
    {"Wave", TextKey::Wave},
    {"StartWave", TextKey::StartWave},
    {"Instruction", TextKey::Instruction},
    {"Tower_Arrow", TextKey::Tower_Arrow},
    {"Tower_Cannon", TextKey::Tower_Cannon},
    {"Tower_Ice", TextKey::Tower_Ice},
    {"Msg_CannotPlace", TextKey::Msg_CannotPlace},
    {"Msg_NoGold", TextKey::Msg_NoGold},
    {"Msg_AlreadyHere", TextKey::Msg_AlreadyHere},
    {"Msg_Placed", TextKey::Msg_Placed},
    {"Msg_Sold", TextKey::Msg_Sold},
    {"Msg_WaveStarted", TextKey::Msg_WaveStarted},
    {"Victory", TextKey::Victory},
    {"GameOver", TextKey::GameOver},
    {"PressR", TextKey::PressR},
    {"EscToMenu", TextKey::EscToMenu},
    {"Back", TextKey::Back},
    {"Volume", TextKey::Volume},
    {"Background", TextKey::Background},
    {"BGM_On", TextKey::BGM_On},
    {"BGM_Off", TextKey::BGM_Off},
    {"CustomMode", TextKey::CustomMode},
    {"CustomWaves", TextKey::CustomWaves},
    {"CustomEnemies", TextKey::CustomEnemies},
    {"CustomGold", TextKey::CustomGold},
    {"CustomLives", TextKey::CustomLives},
    {"CustomSpeed", TextKey::CustomSpeed},
    {"CustomHP", TextKey::CustomHP},
    {"CustomStart", TextKey::CustomStart},
    {"Campaign", TextKey::Campaign},
    {"Level1_Name", TextKey::Level1_Name},
    {"Level1_Desc", TextKey::Level1_Desc},
    {"Level2_Name", TextKey::Level2_Name},
    {"Level2_Desc", TextKey::Level2_Desc},
    {"Level3_Name", TextKey::Level3_Name},
    {"Level3_Desc", TextKey::Level3_Desc},
    {"Biome_Grassland", TextKey::Biome_Grassland},
    {"Biome_Desert", TextKey::Biome_Desert},
    {"Biome_Hell", TextKey::Biome_Hell},
    {"Biome_Community", TextKey::Biome_Community},
    {"L11_Name", TextKey::L11_Name},
    {"L11_Desc", TextKey::L11_Desc},
    {"L12_Name", TextKey::L12_Name},
    {"L12_Desc", TextKey::L12_Desc},
    {"L13_Name", TextKey::L13_Name},
    {"L13_Desc", TextKey::L13_Desc},
    {"L21_Name", TextKey::L21_Name},
    {"L21_Desc", TextKey::L21_Desc},
    {"L22_Name", TextKey::L22_Name},
    {"L22_Desc", TextKey::L22_Desc},
    {"L23_Name", TextKey::L23_Name},
    {"L23_Desc", TextKey::L23_Desc},
    {"L31_Name", TextKey::L31_Name},
    {"L31_Desc", TextKey::L31_Desc},
    {"L32_Name", TextKey::L32_Name},
    {"L32_Desc", TextKey::L32_Desc},
    {"L33_Name", TextKey::L33_Name},
    {"L33_Desc", TextKey::L33_Desc},
    {"C1_Name", TextKey::C1_Name},
    {"C1_Desc", TextKey::C1_Desc},
    {"C2_Name", TextKey::C2_Name},
    {"C2_Desc", TextKey::C2_Desc},

    // 角色系统
    {"CharSelect_Title", TextKey::CharSelect_Title},
    {"CharSelect_New", TextKey::CharSelect_New},
    {"CharSelect_Confirm", TextKey::CharSelect_Confirm},
    {"CharSelect_EnterName", TextKey::CharSelect_EnterName},
    {"CharSelect_Empty", TextKey::CharSelect_Empty},
    {"CharSelect_Delete", TextKey::CharSelect_Delete},
    {"CharSelect_Gold", TextKey::CharSelect_Gold},
    {"CharSelect_Progress", TextKey::CharSelect_Progress},

    // 主菜单（新）
    {"ContinueGame", TextKey::ContinueGame},
    {"NewGame_WarnTitle", TextKey::NewGame_WarnTitle},
    {"NewGame_WarnMsg", TextKey::NewGame_WarnMsg},
    {"NewGame_ConfirmYes", TextKey::NewGame_ConfirmYes},
    {"NewGame_ConfirmNo", TextKey::NewGame_ConfirmNo},

    // 商店
    {"Shop_Title", TextKey::Shop_Title},
    {"Shop_Gold", TextKey::Shop_Gold},
    {"Shop_Buy", TextKey::Shop_Buy},
    {"Shop_Max", TextKey::Shop_Max},
    {"Shop_StartGold", TextKey::Shop_StartGold},
    {"Shop_Discount", TextKey::Shop_Discount},
    {"Shop_Damage", TextKey::Shop_Damage},
    {"Shop_Range", TextKey::Shop_Range},
    {"Shop_Lives", TextKey::Shop_Lives},
    {"Shop_FireRate", TextKey::Shop_FireRate},
    {"Shop_Desc_StartGold", TextKey::Shop_Desc_StartGold},
    {"Shop_Desc_Discount", TextKey::Shop_Desc_Discount},
    {"Shop_Desc_Damage", TextKey::Shop_Desc_Damage},
    {"Shop_Desc_Range", TextKey::Shop_Desc_Range},
    {"Shop_Desc_Lives", TextKey::Shop_Desc_Lives},
    {"Shop_Desc_FireRate", TextKey::Shop_Desc_FireRate},

    // 关卡锁定
    {"Level_Locked", TextKey::Level_Locked},
    {"Level_Complete", TextKey::Level_Complete},

    // 通关界面
    {"NextLevel", TextKey::NextLevel},
    {"BackToMenu", TextKey::BackToMenu},
};

TextKey LangManager::keyFromString(const std::string &name)
{
    for (const auto &entry : KEY_MAP)
    {
        if (name == entry.name)
            return entry.key;
    }
    return TextKey::COUNT; // not found
}

// ============================================================
//  JSON 加载
// ============================================================
bool LangManager::loadLanguage(const std::string &jsonPath)
{
    std::ifstream file(jsonPath);
    if (!file)
    {
        std::cerr << "[LangManager] Cannot open: " << jsonPath << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::unordered_map<std::string, std::string> raw;
    if (!parseFlatJson(content, raw))
    {
        std::cerr << "[LangManager] Failed to parse: " << jsonPath << std::endl;
        return false;
    }

    // 填入 s_texts 数组
    for (const auto &entry : KEY_MAP)
    {
        auto it = raw.find(entry.name);
        if (it != raw.end())
        {
            s_texts[static_cast<int>(entry.key)] = utf8ToWide(it->second);
        }
    }

    // 提取字体路径
    auto fontIt = raw.find("Font");
    if (fontIt != raw.end())
    {
        s_fontPath = fontIt->second;
    }

    // 从文件名提取语言名
    auto pos = jsonPath.find("lang_");
    if (pos != std::string::npos)
    {
        auto end = jsonPath.find(".json", pos);
        s_currentLang = jsonPath.substr(pos + 5, end - pos - 5);
    }

    s_loaded = true;
    std::cout << "[LangManager] Loaded: " << jsonPath << std::endl;
    return true;
}

const std::wstring &LangManager::get(TextKey key)
{
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < static_cast<int>(TextKey::COUNT))
        return s_texts[idx];

    static std::wstring empty;
    return empty;
}

// ============================================================
//  UTF-8 → std::wstring (Windows: UTF-16, Linux: UTF-32)
// ============================================================
#ifdef _WIN32
#include <windows.h>
std::wstring LangManager::utf8ToWide(const std::string &utf8)
{
    if (utf8.empty())
        return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
    result.resize(len - 1); // 去掉 null terminator
    return result;
}
#else
#include <codecvt>
#include <locale>
std::wstring LangManager::utf8ToWide(const std::string &utf8)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(utf8);
}
#endif

// ============================================================
//  轻量平面 JSON 解析器
//  只支持 {"key": "value", ...} 格式，value 必须是字符串
// ============================================================
bool LangManager::parseFlatJson(const std::string &content,
                                std::unordered_map<std::string, std::string> &out)
{
    enum State
    {
        ExpectKey,
        InKey,
        ExpectColon,
        ExpectValue,
        InValue,
        ExpectComma
    };
    State state = ExpectKey;

    std::string key;
    std::string value;
    bool inString = false;
    bool escape = false;

    for (size_t i = 0; i < content.size(); ++i)
    {
        char ch = content[i];

        // 跳过空白
        if (!inString && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
            continue;

        switch (state)
        {
        case ExpectKey:
            if (ch == '}')
                return true; // 空对象
            if (ch == '"')
            {
                key.clear();
                state = InKey;
            }
            else if (ch != '{')
                return false;
            break;

        case InKey:
            if (escape)
            {
                key += ch;
                escape = false;
            }
            else if (ch == '\\')
            {
                escape = true;
            }
            else if (ch == '"')
            {
                state = ExpectColon;
            }
            else
            {
                key += ch;
            }
            break;

        case ExpectColon:
            if (ch == ':')
                state = ExpectValue;
            else
                return false;
            break;

        case ExpectValue:
            if (ch == '"')
            {
                value.clear();
                state = InValue;
            }
            else
                return false;
            break;

        case InValue:
            if (escape)
            {
                value += ch;
                escape = false;
            }
            else if (ch == '\\')
            {
                escape = true;
            }
            else if (ch == '"')
            {
                out[key] = value;
                state = ExpectComma;
            }
            else
            {
                value += ch;
            }
            break;

        case ExpectComma:
            if (ch == ',')
                state = ExpectKey;
            else if (ch == '}')
                return true;
            else
                return false;
            break;
        }
    }

    return state == ExpectComma; // 正常结束于 }
}
