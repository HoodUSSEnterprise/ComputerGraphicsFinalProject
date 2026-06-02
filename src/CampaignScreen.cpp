#include "CampaignScreen.h"
#include "LangManager.h"
#include <iostream>

static const char *biomeNamesC[] = {"grassland", "desert", "hell", "community"};
static const TextKey biomeTitleKeys[] = {
    TextKey::Biome_Grassland, TextKey::Biome_Desert,
    TextKey::Biome_Hell, TextKey::Biome_Community};

CampaignScreen::CampaignScreen()
{
    loadFont();
    buildUI();
}

void CampaignScreen::loadFont()
{
    std::string lang = LangManager::currentLangName();
    std::vector<std::string> paths;
    if (lang == "zh")
        paths = {"fonts/simhei.ttf", "fonts/arial.ttf"};
    else
        paths = {"fonts/arial.ttf"};
    for (const auto &p : paths)
        if (m_font.loadFromFile(p))
        {
            std::cout << "[Font] Campaign loaded: " << p << std::endl;
            break;
        }
}

void CampaignScreen::reloadFont()
{
    loadFont();
    for (auto &row : m_rows)
        row.label.setFont(m_font);
    m_titleText.setFont(m_font);
    m_backHint.setFont(m_font);
    refreshTexts();
}

void CampaignScreen::buildUI()
{
    m_rows.clear();

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(40);
    m_titleText.setFillColor(sf::Color(255, 215, 0));
    m_titleText.setStyle(sf::Text::Bold);

    auto levels = getCampaignLevels();
    int curBiome = -1;
    float y = 110.0f;

    for (size_t i = 0; i < levels.size(); ++i)
    {
        int b = static_cast<int>(levels[i].biome);

        // 新群系标题行
        if (b != curBiome)
        {
            curBiome = b;
            RowItem header;
            header.levelIndex = -1;
            header.bg.setSize(sf::Vector2f(600, 36));
            header.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, y);
            sf::Color hdrCol(60, 60, 80);
            if (b == 0)
                hdrCol = sf::Color(40, 100, 40); // grassland green
            else if (b == 1)
                hdrCol = sf::Color(160, 120, 40); // desert orange
            else if (b == 2)
                hdrCol = sf::Color(140, 30, 30); // hell red
            else if (b == 3)
                hdrCol = sf::Color(60, 60, 120); // community blue
            header.bg.setFillColor(hdrCol);
            header.bg.setOutlineThickness(0);
            header.label.setFont(m_font);
            header.label.setCharacterSize(22);
            header.label.setFillColor(sf::Color::White);
            header.label.setPosition(WINDOW_WIDTH / 2.0f - 290, y + 4);
            m_rows.push_back(header);
            y += 42.0f;
        }

        // 关卡按钮
        RowItem btn;
        btn.levelIndex = static_cast<int>(i);
        btn.bg.setSize(sf::Vector2f(560, 38));
        btn.bg.setPosition(WINDOW_WIDTH / 2.0f - 280, y);
        btn.bg.setFillColor(sf::Color(35, 35, 55));
        btn.bg.setOutlineColor(sf::Color(70, 70, 100));
        btn.bg.setOutlineThickness(1);
        btn.label.setFont(m_font);
        btn.label.setCharacterSize(17);
        btn.label.setFillColor(sf::Color(200, 200, 220));
        btn.label.setPosition(WINDOW_WIDTH / 2.0f - 270, y + 8);
        m_rows.push_back(btn);
        y += 44.0f;
    }

    m_backHint.setFont(m_font);
    m_backHint.setCharacterSize(14);
    m_backHint.setFillColor(sf::Color(120, 120, 140));
    refreshTexts();
}

void CampaignScreen::refreshTexts()
{
    m_titleText.setString(LangManager::get(TextKey::Campaign));
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width / 2, tb.height / 2);
    m_titleText.setPosition(WINDOW_WIDTH / 2.0f, 65);

    auto levels = getCampaignLevels();
    int curBiome = -1;

    for (auto &row : m_rows)
    {
        if (row.levelIndex < 0)
        {
            // 群系标题：从后续关卡推断群系
            for (auto &r2 : m_rows)
            {
                if (r2.levelIndex >= 0)
                {
                    int b = static_cast<int>(levels[r2.levelIndex].biome);
                    if (b > curBiome)
                    {
                        curBiome = b;
                        break;
                    }
                }
            }
            if (curBiome >= 0 && curBiome < 4)
                row.label.setString(LangManager::get(biomeTitleKeys[curBiome]));
        }
        else
        {
            int idx = row.levelIndex;
            if (idx < static_cast<int>(levels.size()))
            {
                row.label.setString(
                    std::wstring(levels[idx].id.begin(), levels[idx].id.end()) + L"  " + LangManager::get(levels[idx].nameKey));
            }
        }
    }

    m_backHint.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
    sf::FloatRect hb = m_backHint.getLocalBounds();
    m_backHint.setOrigin(hb.width / 2, hb.height / 2);
    m_backHint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
}

bool CampaignScreen::update(const sf::Event &event, sf::RenderWindow &window,
                            LevelConfig &outLevel, int unlockedCount)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        auto levels = getCampaignLevels();
        for (auto &row : m_rows)
        {
            bool inside = row.bg.getGlobalBounds().contains(worldPos.x, worldPos.y);
            bool isLevel = row.levelIndex >= 0;
            bool isCommunity = isLevel && row.levelIndex < static_cast<int>(levels.size()) &&
                               levels[row.levelIndex].biome == Biome::Community;
            bool locked = isLevel && row.levelIndex >= unlockedCount && !isCommunity;
            row.hovered = inside && isLevel && !locked;
            if (isLevel)
            {
                if (locked)
                    row.bg.setFillColor(sf::Color(25, 25, 35));
                else
                    row.bg.setFillColor(inside ? sf::Color(55, 55, 90) : sf::Color(35, 35, 55));
            }
        }
        return false;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        int idx = getRowIndex(worldPos.x, worldPos.y);
        if (idx >= 0 && m_rows[idx].levelIndex >= 0)
        {
            int li = m_rows[idx].levelIndex;
            auto levels = getCampaignLevels();
            // 社区关卡不受锁限制
            bool isCommunity = li < static_cast<int>(levels.size()) &&
                               levels[li].biome == Biome::Community;
            if (li >= unlockedCount && !isCommunity)
                return false;
            if (li < static_cast<int>(levels.size()))
            {
                outLevel = levels[li];
                return true;
            }
        }
    }
    return false;
}

void CampaignScreen::draw(sf::RenderWindow &window, int unlockedCount) const
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30, 180));
    window.draw(bg);
    window.draw(m_titleText);

    auto levels = getCampaignLevels();
    for (const auto &row : m_rows)
    {
        window.draw(row.bg);

        bool isCommunity = row.levelIndex >= 0 && row.levelIndex < static_cast<int>(levels.size()) &&
                           levels[row.levelIndex].biome == Biome::Community;
        // 社区关卡不显示锁定/完成标记
        if (!isCommunity && row.levelIndex >= 0 && row.levelIndex >= unlockedCount)
        {
            window.draw(row.label);
            sf::Text lockText;
            lockText.setFont(m_font);
            lockText.setCharacterSize(14);
            lockText.setFillColor(sf::Color(180, 50, 50));
            lockText.setString(LangManager::get(TextKey::Level_Locked));
            lockText.setPosition(row.bg.getPosition().x + row.bg.getSize().x - 80,
                                 row.bg.getPosition().y + 10);
            window.draw(lockText);
        }
        else if (!isCommunity && row.levelIndex >= 0 && row.levelIndex < unlockedCount - 1)
        {
            window.draw(row.label);
            sf::Text doneText;
            doneText.setFont(m_font);
            doneText.setCharacterSize(14);
            doneText.setFillColor(sf::Color(100, 200, 100));
            doneText.setString(LangManager::get(TextKey::Level_Complete));
            doneText.setPosition(row.bg.getPosition().x + row.bg.getSize().x - 80,
                                 row.bg.getPosition().y + 10);
            window.draw(doneText);
        }
        else
        {
            window.draw(row.label);
        }
    }
    window.draw(m_backHint);
}

int CampaignScreen::getRowIndex(float mx, float my) const
{
    for (size_t i = 0; i < m_rows.size(); ++i)
        if (m_rows[i].bg.getGlobalBounds().contains(mx, my))
            return static_cast<int>(i);
    return -1;
}
