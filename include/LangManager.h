#pragma once

#include <string>
#include <unordered_map>

enum class TextKey {
    Title, Subtitle, NewGame, LoadGame, Settings, Exit, MenuHint, Language_Label,
    Gold, Lives, Wave, StartWave, Instruction,
    Tower_Arrow, Tower_Cannon, Tower_Ice,
    Msg_CannotPlace, Msg_NoGold, Msg_AlreadyHere, Msg_Placed, Msg_Sold, Msg_WaveStarted,
    Victory, GameOver, PressR, EscToMenu, Back, Volume, BGM_On, BGM_Off,
    COUNT
};

class LangManager {
public:
    static bool loadLanguage(const std::string& jsonPath);
    static const std::wstring& get(TextKey key);
    static const std::string& currentLangName() { return s_currentLang; }
    static const std::string& getFontPath() { return s_fontPath; }

    static TextKey keyFromString(const std::string& name);

private:
    static bool parseFlatJson(const std::string& content,
                              std::unordered_map<std::string, std::string>& out);
    // UTF-8 → wstring 转换
    static std::wstring utf8ToWide(const std::string& utf8);

    static std::string s_currentLang;
    static std::string s_fontPath;
    static std::wstring s_texts[static_cast<int>(TextKey::COUNT)];
    static bool s_loaded;
};
