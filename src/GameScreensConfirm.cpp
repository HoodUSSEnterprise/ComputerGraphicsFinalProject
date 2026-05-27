// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>

// ============================================================
//  确认对话框
// ============================================================

void Game::buildConfirmUI()
{
    bool zh = (LangManager::currentLangName() == "zh");

    m_confirmTitleText.setFont(m_menuFont);
    m_confirmTitleText.setString(LangManager::get(TextKey::NewGame_WarnTitle));
    m_confirmTitleText.setCharacterSize(zh ? 32 : 36);
    m_confirmTitleText.setFillColor(sf::Color(255, 80, 80));
    m_confirmTitleText.setStyle(sf::Text::Bold);

    m_confirmMsgText.setFont(m_menuFont);
    m_confirmMsgText.setString(LangManager::get(TextKey::NewGame_WarnMsg));
    m_confirmMsgText.setCharacterSize(zh ? 18 : 20);
    m_confirmMsgText.setFillColor(sf::Color(200, 200, 200));

    float bw = zh ? 240.0f : 200.0f;
    m_confirmYesBtn.setSize(sf::Vector2f(bw, 44));
    m_confirmYesBtn.setFillColor(sf::Color(150, 40, 40));
    m_confirmYesBtn.setOutlineColor(sf::Color(255, 100, 100));
    m_confirmYesBtn.setOutlineThickness(2);
    m_confirmYesLabel.setFont(m_menuFont);
    m_confirmYesLabel.setString(LangManager::get(TextKey::NewGame_ConfirmYes));
    m_confirmYesLabel.setCharacterSize(zh ? 16 : 18);
    m_confirmYesLabel.setFillColor(sf::Color::White);

    m_confirmNoBtn.setSize(sf::Vector2f(bw, 44));
    m_confirmNoBtn.setFillColor(sf::Color(50, 50, 70));
    m_confirmNoBtn.setOutlineColor(sf::Color(100, 100, 140));
    m_confirmNoBtn.setOutlineThickness(2);
    m_confirmNoLabel.setFont(m_menuFont);
    m_confirmNoLabel.setString(LangManager::get(TextKey::NewGame_ConfirmNo));
    m_confirmNoLabel.setCharacterSize(zh ? 16 : 18);
    m_confirmNoLabel.setFillColor(sf::Color::White);
}

void Game::renderConfirmDialog()
{
    bool zh = (LangManager::currentLangName() == "zh");

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // 对话框背景
    float dlgW = zh ? 520.0f : 440.0f;
    sf::RectangleShape dlgBg(sf::Vector2f(dlgW, 240));
    dlgBg.setPosition(WINDOW_WIDTH / 2.0f - dlgW / 2, WINDOW_HEIGHT / 2.0f - 120);
    dlgBg.setFillColor(sf::Color(25, 25, 45));
    dlgBg.setOutlineColor(sf::Color(255, 80, 80));
    dlgBg.setOutlineThickness(3);
    m_window.draw(dlgBg);

    sf::FloatRect tb = m_confirmTitleText.getLocalBounds();
    m_confirmTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_confirmTitleText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 80);
    m_window.draw(m_confirmTitleText);

    sf::FloatRect mb = m_confirmMsgText.getLocalBounds();
    m_confirmMsgText.setOrigin(mb.width / 2, mb.height / 2);
    m_confirmMsgText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 20);
    m_window.draw(m_confirmMsgText);

    float bw = zh ? 240.0f : 200.0f;
    float gap = zh ? 30.0f : 20.0f;
    m_confirmYesBtn.setPosition(WINDOW_WIDTH / 2.0f - bw - gap / 2, WINDOW_HEIGHT / 2.0f + 40);
    m_window.draw(m_confirmYesBtn);
    sf::FloatRect yb = m_confirmYesLabel.getLocalBounds();
    m_confirmYesLabel.setOrigin(yb.width / 2, yb.height / 2);
    m_confirmYesLabel.setPosition(WINDOW_WIDTH / 2.0f - bw / 2 - gap / 2, WINDOW_HEIGHT / 2.0f + 62);
    m_window.draw(m_confirmYesLabel);

    m_confirmNoBtn.setPosition(WINDOW_WIDTH / 2.0f + gap / 2, WINDOW_HEIGHT / 2.0f + 40);
    m_window.draw(m_confirmNoBtn);
    sf::FloatRect nb = m_confirmNoLabel.getLocalBounds();
    m_confirmNoLabel.setOrigin(nb.width / 2, nb.height / 2);
    m_confirmNoLabel.setPosition(WINDOW_WIDTH / 2.0f + bw / 2 + gap / 2, WINDOW_HEIGHT / 2.0f + 62);
    m_window.draw(m_confirmNoLabel);
}
