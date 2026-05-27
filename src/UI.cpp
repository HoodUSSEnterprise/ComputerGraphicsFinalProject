#include "UI.h"
#include "LangManager.h"
#include <sstream>
#include <iomanip>
#include <iostream>

UI::UI() : m_messageTimer(0)
{
    reloadFont();

    // 图标纹理
    m_goldTex.loadFromFile("textures/gold.png");
    m_lifeTex.loadFromFile("textures/life.jpg");
    m_goldIcon.setTexture(m_goldTex);
    m_lifeIcon.setTexture(m_lifeTex);

    // 面板背景
    m_panelBg.setSize(sf::Vector2f(WINDOW_WIDTH, 100.0f));
    m_panelBg.setPosition(0, MAP_ROWS * TILE_SIZE);
    m_panelBg.setFillColor(sf::Color(40, 40, 50, 230));
    m_panelBg.setOutlineColor(sf::Color(80, 80, 100));
    m_panelBg.setOutlineThickness(2);

    // 金币文字
    m_goldText.setFont(m_font);
    m_goldText.setCharacterSize(18);
    m_goldText.setFillColor(sf::Color::Yellow);
    m_goldText.setPosition(WINDOW_WIDTH - 275, MAP_ROWS * TILE_SIZE + 6);

    // 生命文字
    m_livesText.setFont(m_font);
    m_livesText.setCharacterSize(18);
    m_livesText.setFillColor(sf::Color::Red);
    m_livesText.setPosition(WINDOW_WIDTH - 275, MAP_ROWS * TILE_SIZE + 26);

    // 波次文字
    m_waveText.setFont(m_font);
    m_waveText.setCharacterSize(18);
    m_waveText.setFillColor(sf::Color::White);
    m_waveText.setPosition(WINDOW_WIDTH - 300, MAP_ROWS * TILE_SIZE + 48);

    // 消息文字
    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(22);
    m_messageText.setFillColor(sf::Color::Yellow);
    m_messageText.setPosition(WINDOW_WIDTH / 2.0f - 150, MAP_ROWS * TILE_SIZE + 70);

    // 操作提示
    m_instructionText.setFont(m_font);
    m_instructionText.setCharacterSize(14);
    m_instructionText.setFillColor(sf::Color(180, 180, 180));
    m_instructionText.setPosition(WINDOW_WIDTH - 300, MAP_ROWS * TILE_SIZE + 75);
    m_instructionText.setString(LangManager::get(TextKey::Instruction));

    // 塔选择按钮
    TextKey nameKeys[] = {TextKey::Tower_Arrow, TextKey::Tower_Cannon, TextKey::Tower_Ice};
    TowerType types[] = {TowerType::Arrow, TowerType::Cannon, TowerType::Ice};

    for (int i = 0; i < 3; ++i)
    {
        TowerButton btn;
        btn.type = types[i];
        btn.bg.setSize(sf::Vector2f(100, 80));
        btn.bg.setPosition(200 + i * 115, MAP_ROWS * TILE_SIZE + 10);
        btn.bg.setOutlineColor(sf::Color::White);
        btn.bg.setOutlineThickness(1);

        TowerStats s = Tower::getStats(types[i]);
        btn.bg.setFillColor(s.color - sf::Color(80, 80, 80, 0));

        btn.label.setFont(m_font);
        btn.label.setCharacterSize(16);
        btn.label.setFillColor(sf::Color::White);
        btn.label.setString(LangManager::get(nameKeys[i]));
        btn.label.setPosition(210 + i * 115, MAP_ROWS * TILE_SIZE + 12);

        btn.cost.setFont(m_font);
        btn.cost.setCharacterSize(14);
        btn.cost.setFillColor(sf::Color::Yellow);
        btn.cost.setString("$" + std::to_string(s.cost));
        btn.cost.setPosition(210 + i * 115, MAP_ROWS * TILE_SIZE + 55);

        m_towerButtons.push_back(btn);
    }

    // 开始波次按钮（左下）
    m_startWaveBtn.setSize(sf::Vector2f(130, 50));
    m_startWaveBtn.setPosition(15, MAP_ROWS * TILE_SIZE + 25);
    m_startWaveBtn.setFillColor(sf::Color(50, 150, 50));
    m_startWaveBtn.setOutlineColor(sf::Color::White);
    m_startWaveBtn.setOutlineThickness(2);

    m_startWaveText.setFont(m_font);
    m_startWaveText.setCharacterSize(18);
    m_startWaveText.setFillColor(sf::Color::White);
    m_startWaveText.setString(LangManager::get(TextKey::StartWave));
    m_startWaveText.setPosition(28, MAP_ROWS * TILE_SIZE + 38);
}

void UI::update(float dt, int gold, int lives, int currentWave,
                int totalWaves, TowerType selectedTower, float waveCountdown)
{

    // 更新文字
    m_goldText.setString(std::to_wstring(gold));
    m_livesText.setString(std::to_wstring(lives));
    m_waveText.setString(std::wstring(LangManager::get(TextKey::Wave)) + L": " + std::to_wstring(currentWave) + L" / " + std::to_wstring(totalWaves));

    // 更新波次按钮：显示倒计时或"开始出怪"
    if (waveCountdown > 0)
    {
        m_startWaveText.setString(std::to_wstring(static_cast<int>(waveCountdown + 0.999f)) + L"s");
    }
    else
    {
        m_startWaveText.setString(LangManager::get(TextKey::StartWave));
    }

    // 更新消息
    if (m_messageTimer > 0)
    {
        m_messageTimer -= dt;
    }
    else
    {
        m_messageText.setString("");
    }

    // 更新按钮高亮
    for (auto &btn : m_towerButtons)
    {
        if (btn.type == selectedTower)
        {
            btn.bg.setOutlineColor(sf::Color::Yellow);
            btn.bg.setOutlineThickness(3);
        }
        else
        {
            btn.bg.setOutlineColor(sf::Color::White);
            btn.bg.setOutlineThickness(1);
        }
    }
}

void UI::draw(sf::RenderWindow &window) const
{
    window.draw(m_panelBg);

    // 金币图标
    float gy = MAP_ROWS * TILE_SIZE + 6;
    m_goldIcon.setPosition(WINDOW_WIDTH - 300, gy);
    m_goldIcon.setScale(20.0f / m_goldTex.getSize().x, 20.0f / m_goldTex.getSize().y);
    window.draw(m_goldIcon);

    // 生命图标
    float ly = MAP_ROWS * TILE_SIZE + 26;
    m_lifeIcon.setPosition(WINDOW_WIDTH - 300, ly);
    m_lifeIcon.setScale(20.0f / m_lifeTex.getSize().x, 20.0f / m_lifeTex.getSize().y);
    window.draw(m_lifeIcon);

    window.draw(m_goldText);
    window.draw(m_livesText);
    window.draw(m_waveText);
    window.draw(m_messageText);
    window.draw(m_instructionText);

    window.draw(m_startWaveBtn);
    window.draw(m_startWaveText);
}

TowerType UI::handleClick(float x, float y) const
{
    for (const auto &btn : m_towerButtons)
    {
        if (btn.bg.getGlobalBounds().contains(x, y))
        {
            return btn.type;
        }
    }
    return TowerType::Arrow; // 默认
}

bool UI::isClickOnUI(float x, float y) const
{
    return y >= MAP_ROWS * TILE_SIZE;
}

bool UI::isStartWaveClicked(float x, float y) const
{
    return m_startWaveBtn.getGlobalBounds().contains(x, y);
}

void UI::showMessage(const std::wstring &msg)
{
    m_messageText.setString(msg);
    m_messageTimer = 2.0f;
}

void UI::reloadFont()
{
    std::vector<std::string> paths = {LangManager::getFontPath()};

    std::string lang = LangManager::currentLangName();
    if (lang == "zh")
    {
        paths.push_back("fonts/simhei.ttf");
    }
    paths.push_back("fonts/arial.ttf");

    bool loaded = false;
    for (const auto &p : paths)
    {
        if (m_font.loadFromFile(p))
        {
            std::cout << "[Font] UI loaded: " << p << std::endl;
            loaded = true;
            break;
        }
    }
    if (!loaded)
        std::cerr << "[Font] WARNING: UI font failed to load!" << std::endl;

    m_goldText.setFont(m_font);
    m_livesText.setFont(m_font);
    m_waveText.setFont(m_font);
    m_messageText.setFont(m_font);
    m_instructionText.setFont(m_font);
    m_startWaveText.setFont(m_font);
    for (auto &btn : m_towerButtons)
    {
        btn.label.setFont(m_font);
        btn.cost.setFont(m_font);
    }

    refreshTexts();
}

void UI::refreshTexts()
{
    m_instructionText.setString(LangManager::get(TextKey::Instruction));
    m_startWaveText.setString(LangManager::get(TextKey::StartWave));

    TextKey nameKeys[] = {TextKey::Tower_Arrow, TextKey::Tower_Cannon, TextKey::Tower_Ice};
    for (size_t i = 0; i < m_towerButtons.size() && i < 3; ++i)
    {
        m_towerButtons[i].label.setString(LangManager::get(nameKeys[i]));
    }
}
