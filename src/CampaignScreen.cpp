#include "CampaignScreen.h"
#include "LangManager.h"
#include <iostream>

CampaignScreen::CampaignScreen()
{
    loadFont();
    buildUI();
}

void CampaignScreen::loadFont()
{
    std::vector<std::string> paths = {LangManager::getFontPath()};
    std::string lang = LangManager::currentLangName();
    if (lang == "zh")
        paths.push_back("fonts/simhei.ttf");
    paths.push_back("fonts/arial.ttf");

    for (const auto &p : paths)
    {
        if (m_font.loadFromFile(p))
        {
            std::cout << "[Font] CampaignScreen loaded: " << p << std::endl;
            break;
        }
    }
}

void CampaignScreen::buildUI()
{
    m_buttons.clear();

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(46);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);

    auto levels = getCampaignLevels();
    for (size_t i = 0; i < levels.size(); ++i)
    {
        LevelButton btn;
        float yPos = 160.0f + i * 160.0f;

        btn.bg.setSize(sf::Vector2f(600, 130));
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, yPos);
        btn.bg.setFillColor(sf::Color(40, 40, 60));
        btn.bg.setOutlineColor(sf::Color(80, 80, 120));
        btn.bg.setOutlineThickness(2);

        // 关卡名称
        btn.nameText.setFont(m_font);
        btn.nameText.setCharacterSize(28);
        btn.nameText.setFillColor(sf::Color(255, 215, 0));
        btn.nameText.setPosition(WINDOW_WIDTH / 2.0f - 280, yPos + 10);

        // 描述
        btn.descText.setFont(m_font);
        btn.descText.setCharacterSize(16);
        btn.descText.setFillColor(sf::Color(180, 180, 200));
        btn.descText.setPosition(WINDOW_WIDTH / 2.0f - 280, yPos + 50);

        // 参数信息
        btn.infoText.setFont(m_font);
        btn.infoText.setCharacterSize(14);
        btn.infoText.setFillColor(sf::Color(140, 200, 140));
        btn.infoText.setPosition(WINDOW_WIDTH / 2.0f - 280, yPos + 85);

        m_buttons.push_back(btn);
    }

    m_backHint.setFont(m_font);
    m_backHint.setCharacterSize(14);
    m_backHint.setFillColor(sf::Color(120, 120, 140));
    m_backHint.setString("ESC: Back to Menu");

    refreshTexts();
}

void CampaignScreen::refreshTexts()
{
    auto levels = getCampaignLevels();
    m_titleText.setString(LangManager::get(TextKey::Campaign));
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 90);

    for (size_t i = 0; i < m_buttons.size() && i < levels.size(); ++i)
    {
        m_buttons[i].nameText.setString(
            std::to_wstring(i + 1) + L". " + LangManager::get(levels[i].nameKey));
        m_buttons[i].descText.setString(LangManager::get(levels[i].descKey));

        std::wstring info = L"$" + std::to_wstring(levels[i].startGold) + L"  |  " + std::to_wstring(levels[i].startLives) + L" HP" + L"  |  " + std::to_wstring(levels[i].waveCount) + L" " + LangManager::get(TextKey::Wave);
        m_buttons[i].infoText.setString(info);
    }

    sf::FloatRect hb = m_backHint.getLocalBounds();
    m_backHint.setOrigin(hb.width / 2, hb.height / 2);
    m_backHint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
}

bool CampaignScreen::update(const sf::Event &event, sf::RenderWindow &window,
                            LevelConfig &outLevel)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (auto &btn : m_buttons)
        {
            bool inside = btn.bg.getGlobalBounds().contains(worldPos.x, worldPos.y);
            btn.hovered = inside;
            btn.bg.setFillColor(inside ? sf::Color(60, 60, 90) : sf::Color(40, 40, 60));
            btn.bg.setOutlineColor(inside ? sf::Color(255, 215, 0) : sf::Color(80, 80, 120));
        }
        return false;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        int idx = getButtonIndex(worldPos.x, worldPos.y);
        if (idx >= 0 && idx < static_cast<int>(m_buttons.size()))
        {
            outLevel = getCampaignLevels()[idx];
            return true;
        }
    }

    return false;
}

void CampaignScreen::draw(sf::RenderWindow &window) const
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30));
    window.draw(bg);

    for (int i = 0; i < 10; ++i)
    {
        sf::RectangleShape line(sf::Vector2f(WINDOW_WIDTH, 2));
        line.setFillColor(sf::Color(30, 30, 50));
        line.setPosition(0, i * 80.0f);
        window.draw(line);
    }

    window.draw(m_titleText);
    for (const auto &btn : m_buttons)
    {
        window.draw(btn.bg);
        window.draw(btn.nameText);
        window.draw(btn.descText);
        window.draw(btn.infoText);
    }
    window.draw(m_backHint);
}

int CampaignScreen::getButtonIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    }
    return -1;
}
