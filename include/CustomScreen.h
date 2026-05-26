#pragma once

#include "LevelData.h"
#include <SFML/Graphics.hpp>
#include <vector>

// 自定义模式参数设置界面
class CustomScreen
{
public:
    CustomScreen();

    // 返回 true 表示玩家点击了开始，params 被填充
    bool update(const sf::Event &event, sf::RenderWindow &window,
                CustomParams &outParams);
    void draw(sf::RenderWindow &window) const;
    void refreshTexts();

private:
    struct ParamButton
    {
        sf::RectangleShape bg;
        sf::Text label;
        bool hovered = false;
    };

    sf::Font m_font;
    CustomParams m_params;
    std::vector<ParamButton> m_buttons;
    sf::Text m_titleText;
    sf::Text m_backHint;

    int getButtonIndex(float mx, float my) const;
    void loadFont();
    void buildUI();
    void refreshValueLabels();
};
