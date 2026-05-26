#pragma once

#include "LevelData.h"
#include <SFML/Graphics.hpp>
#include <vector>

// 战役选关界面：展示3个关卡，玩家选择后返回对应 LevelConfig
class CampaignScreen {
public:
    CampaignScreen();

    // 返回 true 表示玩家选择了关卡，level 被填充
    bool update(const sf::Event& event, sf::RenderWindow& window,
                LevelConfig& outLevel);
    void draw(sf::RenderWindow& window) const;
    void refreshTexts();

private:
    struct LevelButton {
        sf::RectangleShape bg;
        sf::Text nameText;
        sf::Text descText;
        sf::Text infoText;
        bool hovered = false;
    };

    sf::Font m_font;
    sf::Text m_titleText;
    std::vector<LevelButton> m_buttons;
    sf::Text m_backHint;

    int getButtonIndex(float mx, float my) const;
    void loadFont();
    void buildUI();
};
