#pragma once

#include "Constants.h"
#include "Tower.h"
#include <SFML/Graphics.hpp>
#include <vector>

class UI {
public:
    UI();

    void update(float dt, int gold, int lives, int currentWave,
                int totalWaves, TowerType selectedTower);
    void draw(sf::RenderWindow& window) const;

    // 检查是否点击了 UI 面板
    TowerType handleClick(float x, float y) const;
    bool isClickOnUI(float x, float y) const;
    bool isStartWaveClicked(float x, float y) const;

    void showMessage(const std::wstring& msg);
    void reloadFont();
    void refreshTexts();

private:
    sf::Font m_font;
    sf::Text m_goldText;
    sf::Text m_livesText;
    sf::Text m_waveText;
    sf::Text m_messageText;
    float m_messageTimer;
    sf::RectangleShape m_panelBg;

    // 塔选择按钮
    struct TowerButton {
        sf::RectangleShape bg;
        sf::Text label;
        sf::Text cost;
        TowerType type;
    };
    std::vector<TowerButton> m_towerButtons;

    sf::RectangleShape m_startWaveBtn;
    sf::Text m_startWaveText;

    sf::Text m_instructionText;
};
