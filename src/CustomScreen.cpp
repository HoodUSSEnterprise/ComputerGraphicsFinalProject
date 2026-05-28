#include "CustomScreen.h"
#include "LangManager.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>

CustomScreen::CustomScreen() : m_params()
{
    loadFont();
    scanMaps();
    buildUI();
}

void CustomScreen::scanMaps()
{
    m_mapList.clear();
    namespace fs = std::filesystem;
    std::error_code ec;

    auto addDir = [&](const std::string &dir) {
        if (!fs::exists(dir, ec)) return;
        for (const auto &entry : fs::directory_iterator(dir, ec))
        {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() == ".txt")
                m_mapList.push_back(entry.path().string());
        }
    };

    addDir("../assets/maps/grassland");
    addDir("../assets/maps/desert");
    addDir("../assets/maps/hell");
    addDir("../assets/maps/community");
    // 也扫描自定义编辑器地图
    std::string customPath = "../assets/maps/custom_editor.txt";
    if (fs::exists(customPath, ec))
        m_mapList.push_back(customPath);

    if (m_mapList.empty())
        m_mapList.push_back("../assets/maps/grassland/1-1.txt");
    m_params.mapFile = m_mapList[0];
}

void CustomScreen::loadFont()
{
    std::string lang = LangManager::currentLangName();
    std::vector<std::string> paths;
    if (lang == "zh")
        paths = {"fonts/simhei.ttf", "fonts/arial.ttf"};
    else
        paths = {"fonts/arial.ttf"};

    for (const auto &p : paths)
    {
        if (m_font.loadFromFile(p))
        {
            std::cout << "[Font] CustomScreen loaded: " << p << std::endl;
            break;
        }
    }
}

void CustomScreen::buildUI()
{
    m_buttons.clear();

    // 加载按钮纹理
    m_btnPlusTex.loadFromFile("textures/ButtonPlus.png");
    m_btnMinusTex.loadFromFile("textures/ButtonMinus.png");
    m_arrowLeftTex.loadFromFile("textures/ArrowLeft.png");
    m_arrowRightTex.loadFromFile("textures/ArrowRight.png");
    m_buttonTex.loadFromFile("textures/button.png");

    // 地图选择行
    {
        float yPos = 140.0f;
        ParamButton labelBtn;
        labelBtn.bg.setSize(sf::Vector2f(180, 40));
        labelBtn.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, yPos);
        labelBtn.bg.setFillColor(sf::Color(40, 40, 55));
        labelBtn.label.setFont(m_font);
        labelBtn.label.setString("Map");
        labelBtn.label.setCharacterSize(20);
        labelBtn.label.setFillColor(sf::Color(180, 180, 200));
        labelBtn.label.setPosition(WINDOW_WIDTH / 2.0f - 295, yPos + 6);
        m_buttons.push_back(labelBtn);

        // 左箭头
        ParamButton leftBtn;
        leftBtn.bg.setSize(sf::Vector2f(50, 40));
        leftBtn.bg.setPosition(WINDOW_WIDTH / 2.0f - 100, yPos);
        leftBtn.bg.setTexture(&m_arrowLeftTex);
        leftBtn.label.setFont(m_font);
        leftBtn.label.setString("");
        m_buttons.push_back(leftBtn);

        // 地图名
        ParamButton nameBtn;
        nameBtn.bg.setSize(sf::Vector2f(200, 40));
        nameBtn.bg.setPosition(WINDOW_WIDTH / 2.0f - 45, yPos);
        nameBtn.bg.setFillColor(sf::Color(30, 30, 45));
        nameBtn.bg.setOutlineColor(sf::Color(80, 80, 100));
        nameBtn.bg.setOutlineThickness(1);
        nameBtn.label.setFont(m_font);
        nameBtn.label.setCharacterSize(16);
        nameBtn.label.setFillColor(sf::Color::Yellow);
        m_buttons.push_back(nameBtn);

        // 右箭头
        ParamButton rightBtn;
        rightBtn.bg.setSize(sf::Vector2f(50, 40));
        rightBtn.bg.setPosition(WINDOW_WIDTH / 2.0f + 160, yPos);
        rightBtn.bg.setTexture(&m_arrowRightTex);
        rightBtn.label.setFont(m_font);
        rightBtn.label.setString("");
        m_buttons.push_back(rightBtn);
    }

    struct ParamDef
    {
        TextKey labelKey;
        int *valueInt;
        float *valueFloat;
        int minInt, maxInt;
        float minFloat, maxFloat;
        float step;
        bool isFloat;
    };

    ParamDef params[] = {
        {TextKey::CustomWaves, &m_params.waves, nullptr, 1, 20, 0, 0, 1, false},
        {TextKey::CustomEnemies, &m_params.enemiesPerWave, nullptr, 5, 50, 0, 0, 1, false},
        {TextKey::CustomGold, &m_params.startGold, nullptr, 100, 1000, 0, 0, 50, false},
        {TextKey::CustomLives, &m_params.startLives, nullptr, 5, 100, 0, 0, 5, false},
        {TextKey::CustomSpeed, nullptr, &m_params.speedMul, 0, 0, 0.5f, 3.0f, 0.1f, true},
        {TextKey::CustomHP, nullptr, &m_params.hpMul, 0, 0, 0.5f, 3.0f, 0.1f, true},
    };
    constexpr int paramCount = 6;

    for (int i = 0; i < paramCount; ++i)
    {
        float yPos = 230.0f + i * 60.0f;

        // 标签
        {
            ParamButton btn;
            btn.bg.setSize(sf::Vector2f(180, 40));
            btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, yPos);
            btn.bg.setFillColor(sf::Color(40, 40, 55));
            btn.bg.setOutlineThickness(0);
            btn.label.setFont(m_font);
            btn.label.setString(LangManager::get(params[i].labelKey));
            btn.label.setCharacterSize(20);
            btn.label.setFillColor(sf::Color(180, 180, 200));
            btn.label.setPosition(WINDOW_WIDTH / 2.0f - 295, yPos + 6);
            m_buttons.push_back(btn);
        }

        // 减号按钮
        {
            ParamButton btn;
            btn.bg.setSize(sf::Vector2f(50, 40));
            btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 100, yPos);
            btn.bg.setTexture(&m_btnMinusTex);
            btn.bg.setOutlineThickness(0);
            btn.label.setFont(m_font);
            btn.label.setString("");
            m_buttons.push_back(btn);
        }

        // 值标签
        {
            ParamButton btn;
            btn.bg.setSize(sf::Vector2f(120, 40));
            btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 45, yPos);
            btn.bg.setFillColor(sf::Color(30, 30, 45));
            btn.bg.setOutlineColor(sf::Color(80, 80, 100));
            btn.bg.setOutlineThickness(1);
            btn.label.setFont(m_font);
            btn.label.setCharacterSize(20);
            btn.label.setFillColor(sf::Color::Yellow);
            m_buttons.push_back(btn);
        }

        // 加号按钮
        {
            ParamButton btn;
            btn.bg.setSize(sf::Vector2f(50, 40));
            btn.bg.setPosition(WINDOW_WIDTH / 2.0f + 80, yPos);
            btn.bg.setTexture(&m_btnPlusTex);
            btn.bg.setOutlineThickness(0);
            btn.label.setFont(m_font);
            btn.label.setString("");
            m_buttons.push_back(btn);
        }
    }

    // 开始按钮
    {
        ParamButton btn;
        btn.bg.setSize(sf::Vector2f(250, 50));
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 125, 625);
        btn.bg.setTexture(&m_buttonTex);
        btn.bg.setFillColor(sf::Color(180, 255, 180));
        btn.bg.setOutlineThickness(0);
        btn.label.setFont(m_font);
        btn.label.setString(LangManager::get(TextKey::CustomStart));
        btn.label.setCharacterSize(24);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 650);
        m_buttons.push_back(btn);
    }

    // 返回按钮
    {
        ParamButton btn;
        btn.bg.setSize(sf::Vector2f(200, 50));
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 100, 695);
        btn.bg.setTexture(&m_buttonTex);
        btn.bg.setFillColor(sf::Color(200, 200, 220));
        btn.bg.setOutlineThickness(0);
        btn.label.setFont(m_font);
        btn.label.setString(LangManager::get(TextKey::Back));
        btn.label.setCharacterSize(22);
        btn.label.setFillColor(sf::Color::White);
        sf::FloatRect lb = btn.label.getLocalBounds();
        btn.label.setOrigin(lb.width / 2, lb.height / 2);
        btn.label.setPosition(WINDOW_WIDTH / 2.0f, 720);
        m_buttons.push_back(btn);
    }

    m_backHint.setFont(m_font);
    m_backHint.setCharacterSize(14);
    m_backHint.setFillColor(sf::Color(120, 120, 140));
    m_backHint.setString("ESC: Back to Menu");

    refreshValueLabels();
    refreshTexts();
}

void CustomScreen::refreshValueLabels()
{
    struct
    {
        int intVal;
        float floatVal;
        bool isFloat;
    } vals[] = {
        {m_params.waves, 0, false},
        {m_params.enemiesPerWave, 0, false},
        {m_params.startGold, 0, false},
        {m_params.startLives, 0, false},
        {0, m_params.speedMul, true},
        {0, m_params.hpMul, true},
    };

    for (int i = 0; i < 6; ++i)
    {
        int idx = i * 4 + 2 + 4;  // +4 for map row
        if (idx < static_cast<int>(m_buttons.size()))
        {
            if (vals[i].isFloat)
            {
                std::wstringstream wss;
                wss << L"x" << std::fixed << std::setprecision(1) << vals[i].floatVal;
                m_buttons[idx].label.setString(wss.str());
            }
            else
            {
                m_buttons[idx].label.setString(std::to_wstring(vals[i].intVal));
            }
            sf::FloatRect lb = m_buttons[idx].label.getLocalBounds();
            float bx = m_buttons[idx].bg.getPosition().x;
            float bw = m_buttons[idx].bg.getSize().x;
            m_buttons[idx].label.setPosition(
                bx + bw / 2.0f - lb.width / 2,
                m_buttons[idx].bg.getPosition().y + 6);
        }
    }
}

void CustomScreen::refreshTexts()
{
    m_titleText.setFont(m_font);
    m_titleText.setString(LangManager::get(TextKey::CustomMode));
    m_titleText.setCharacterSize(46);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 90);

    // 刷新参数标签 (每4个按钮一组: 0=标签, 1=-, 2=值, 3=+)
    static const TextKey paramKeys[] = {
        TextKey::CustomWaves, TextKey::CustomEnemies,
        TextKey::CustomGold, TextKey::CustomLives,
        TextKey::CustomSpeed, TextKey::CustomHP
    };
    for (int i = 0; i < 6; ++i) {
        int idx = i * 4 + 4;  // +4 for map row
        if (idx < static_cast<int>(m_buttons.size()))
            m_buttons[idx].label.setString(LangManager::get(paramKeys[i]));
    }

    // 刷新开始/返回按钮
    int n = static_cast<int>(m_buttons.size());
    if (n > 28) {
        m_buttons[28].label.setString(LangManager::get(TextKey::CustomStart));
        sf::FloatRect lb = m_buttons[28].label.getLocalBounds();
        m_buttons[28].label.setOrigin(lb.width / 2, lb.height / 2);
        m_buttons[28].label.setPosition(WINDOW_WIDTH / 2.0f, 650);
    }
    if (n > 29) {
        m_buttons[29].label.setString(LangManager::get(TextKey::Back));
        sf::FloatRect lb = m_buttons[29].label.getLocalBounds();
        m_buttons[29].label.setOrigin(lb.width / 2, lb.height / 2);
        m_buttons[29].label.setPosition(WINDOW_WIDTH / 2.0f, 720);
    }

    // 地图名（居中在显示框里）
    if (!m_mapList.empty())
    {
        std::string name = m_mapList[m_mapIndex];
        auto pos = name.rfind('/');
        if (pos != std::string::npos) name = name.substr(pos + 1);
        pos = name.rfind('\\');
        if (pos != std::string::npos) name = name.substr(pos + 1);
        m_buttons[2].label.setString(std::wstring(name.begin(), name.end()));
        sf::FloatRect lb = m_buttons[2].label.getLocalBounds();
        float bx = m_buttons[2].bg.getPosition().x;
        float bw = m_buttons[2].bg.getSize().x;
        m_buttons[2].label.setPosition(
            bx + bw / 2.0f - lb.width / 2,
            m_buttons[2].bg.getPosition().y + 8);
    }

    m_backHint.setString(L"ESC: Back to Menu");
    sf::FloatRect hb = m_backHint.getLocalBounds();
    m_backHint.setOrigin(hb.width / 2, hb.height / 2);
    m_backHint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
}

void CustomScreen::reloadFont()
{
    loadFont();
    for (auto &btn : m_buttons)
        btn.label.setFont(m_font);
    m_titleText.setFont(m_font);
    m_backHint.setFont(m_font);
    refreshTexts();
}

int CustomScreen::update(const sf::Event &event, sf::RenderWindow &window,
                          CustomParams &outParams)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (auto &btn : m_buttons)
        {
            bool inside = btn.bg.getGlobalBounds().contains(worldPos.x, worldPos.y);
            btn.hovered = inside;
            if (inside)
                btn.bg.setOutlineColor(sf::Color(255, 215, 0));
            else if (btn.bg.getOutlineThickness() > 0)
                btn.bg.setOutlineColor(sf::Color(100, 100, 140));
        }
        return 0;
    }

    if (event.type != sf::Event::MouseButtonPressed ||
        event.mouseButton.button != sf::Mouse::Left)
        return 0;

    sf::Vector2f worldPos = window.mapPixelToCoords(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    int idx = getButtonIndex(worldPos.x, worldPos.y);
    if (idx < 0) return 0;

    // 地图选择行 (0-3)
    if (idx >= 0 && idx <= 3)
    {
        if (idx == 1 && !m_mapList.empty()) // 左箭头
        {
            m_mapIndex = (m_mapIndex - 1 + m_mapList.size()) % m_mapList.size();
            m_params.mapFile = m_mapList[m_mapIndex];
            refreshTexts();
        }
        else if (idx == 3 && !m_mapList.empty()) // 右箭头
        {
            m_mapIndex = (m_mapIndex + 1) % m_mapList.size();
            m_params.mapFile = m_mapList[m_mapIndex];
            refreshTexts();
        }
        return 0;
    }

    // 参数行 (偏移4)
    int paramBase = idx - 4;
    int paramIdx = paramBase / 4;
    int btnType = paramBase % 4;

    if (paramIdx < 6)
    {
        switch (btnType)
        {
        case 1: // -
            switch (paramIdx)
            {
            case 0:
                m_params.waves = std::max(1, m_params.waves - 1);
                break;
            case 1:
                m_params.enemiesPerWave = std::max(5, m_params.enemiesPerWave - 1);
                break;
            case 2:
                m_params.startGold = std::max(100, m_params.startGold - 50);
                break;
            case 3:
                m_params.startLives = std::max(5, m_params.startLives - 5);
                break;
            case 4:
                m_params.speedMul = std::max(0.5f, m_params.speedMul - 0.1f);
                break;
            case 5:
                m_params.hpMul = std::max(0.5f, m_params.hpMul - 0.1f);
                break;
            }
            refreshValueLabels();
            break;
        case 3: // +
            switch (paramIdx)
            {
            case 0:
                m_params.waves = std::min(20, m_params.waves + 1);
                break;
            case 1:
                m_params.enemiesPerWave = std::min(50, m_params.enemiesPerWave + 1);
                break;
            case 2:
                m_params.startGold = std::min(1000, m_params.startGold + 50);
                break;
            case 3:
                m_params.startLives = std::min(100, m_params.startLives + 5);
                break;
            case 4:
                m_params.speedMul = std::min(3.0f, m_params.speedMul + 0.1f);
                break;
            case 5:
                m_params.hpMul = std::min(3.0f, m_params.hpMul + 0.1f);
                break;
            }
            refreshValueLabels();
            break;
        default:
            break;
        }
    }
    else if (idx == 28)
    {
        outParams = m_params;
        return 1; // 开始
    }
    else if (idx == 29)
    {
        return 2; // 返回
    }

    return 0;
}

void CustomScreen::draw(sf::RenderWindow &window) const
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    window.draw(bg);

    window.draw(m_titleText);
    for (const auto &btn : m_buttons)
    {
        window.draw(btn.bg);
        window.draw(btn.label);
    }
    window.draw(m_backHint);
}

int CustomScreen::getButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    }
    return -1;
}
