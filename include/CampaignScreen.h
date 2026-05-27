#pragma once

#include "LevelData.h"
#include <SFML/Graphics.hpp>
#include <vector>

// 战役选关界面：群系分组，子关卡可选
class CampaignScreen
{
public:
    CampaignScreen();

    bool update(const sf::Event &event, sf::RenderWindow &window,
                LevelConfig &outLevel, int unlockedCount);
    void draw(sf::RenderWindow &window, int unlockedCount) const;
    void refreshTexts();
    void reloadFont();

private:
    // 每个UI行：可能是群系标题，也可能是关卡按钮
    struct RowItem
    {
        sf::RectangleShape bg;
        sf::Text label;
        int levelIndex = -1;  // -1 = 群系标题, >=0 = 关卡在 getCampaignLevels() 中的索引
        bool hovered = false;
    };

    sf::Font m_font;
    sf::Text m_titleText;
    std::vector<RowItem> m_rows;
    sf::Text m_backHint;

    int getRowIndex(float mx, float my) const;
    void loadFont();
    void buildUI();
};
