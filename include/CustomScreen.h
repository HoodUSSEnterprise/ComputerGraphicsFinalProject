#pragma once

#include "LevelData.h"
#include <SFML/Graphics.hpp>
#include <vector>

// 自定义模式参数设置界面
class CustomScreen
{
public:
    CustomScreen();

    // 返回: 0=无操作, 1=开始游戏(params已填充), 2=返回
    int update(const sf::Event &event, sf::RenderWindow &window,
               CustomParams &outParams);
    void draw(sf::RenderWindow &window) const;
    void refreshTexts();
    void reloadFont();

private:
    struct ParamButton
    {
        sf::RectangleShape bg;
        sf::Text label;
        bool hovered = false;
    };

    sf::Font m_font;
    sf::Texture m_btnPlusTex;
    sf::Texture m_btnMinusTex;
    sf::Texture m_arrowLeftTex;
    sf::Texture m_arrowRightTex;
    sf::Texture m_buttonTex;
    CustomParams m_params;
    std::vector<ParamButton> m_buttons;
    sf::Text m_titleText;
    sf::Text m_backHint;

    // 地图选择
    std::vector<std::string> m_mapList;
    int m_mapIndex = 0;

    int getButtonIndex(float mx, float my) const;
    void loadFont();
    void buildUI();
    void refreshValueLabels();
    void scanMaps();
};
