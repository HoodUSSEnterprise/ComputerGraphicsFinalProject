// GameScreens.cpp — Game 的菜单/设置/战役/自定义界面实现
#include "Game.h"
#include "LangManager.h"
#include <iostream>
// ============================================================
//  商店界面
// ============================================================

void Game::buildShopUI()
{
    m_shopButtons.clear();

    m_shopTitleText.setFont(m_menuFont);
    m_shopTitleText.setString(LangManager::get(TextKey::Shop_Title));
    m_shopTitleText.setCharacterSize(44);
    m_shopTitleText.setFillColor(sf::Color(255, 215, 0));
    m_shopTitleText.setStyle(sf::Text::Bold);

    m_shopGoldText.setFont(m_menuFont);
    m_shopGoldText.setCharacterSize(20);
    m_shopGoldText.setFillColor(sf::Color(255, 215, 0));

    m_shopBackHint.setFont(m_menuFont);
    m_shopBackHint.setString(L"ESC: " + std::wstring(LangManager::get(TextKey::Back)));
    m_shopBackHint.setCharacterSize(14);
    m_shopBackHint.setFillColor(sf::Color(120, 120, 140));

    // 6个商店项目
    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        ShopItem item = static_cast<ShopItem>(i);
        ShopButton sb;
        float y = 180.0f + i * 70.0f;

        sb.bg.setSize(sf::Vector2f(600, 56));
        sb.bg.setPosition(WINDOW_WIDTH / 2.0f - 300, y);
        sb.bg.setFillColor(sf::Color(30, 30, 50));
        sb.bg.setOutlineColor(sf::Color(70, 70, 100));
        sb.bg.setOutlineThickness(1);

        sb.nameText.setFont(m_menuFont);
        sb.nameText.setCharacterSize(20);
        sb.nameText.setFillColor(sf::Color::White);

        sb.levelText.setFont(m_menuFont);
        sb.levelText.setCharacterSize(16);
        sb.levelText.setFillColor(sf::Color(180, 180, 200));

        sb.costText.setFont(m_menuFont);
        sb.costText.setCharacterSize(16);
        sb.costText.setFillColor(sf::Color(255, 215, 0));

        sb.buyBtn.setSize(sf::Vector2f(100, 34));
        sb.buyBtn.setFillColor(sf::Color(40, 100, 40));
        sb.buyBtn.setOutlineColor(sf::Color(100, 200, 100));
        sb.buyBtn.setOutlineThickness(1);

        sb.buyLabel.setFont(m_menuFont);
        sb.buyLabel.setCharacterSize(16);
        sb.buyLabel.setFillColor(sf::Color::White);
        sb.buyLabel.setString(LangManager::get(TextKey::Shop_Buy));

        m_shopButtons.push_back(sb);
    }

    refreshShopTexts();
}

void Game::refreshShopTexts()
{
    m_shopGoldText.setString(
        std::wstring(LangManager::get(TextKey::Shop_Gold)) + L": " +
        std::to_wstring(m_playerData.totalGold));

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        auto &sb = m_shopButtons[i];
        ShopItem item = static_cast<ShopItem>(i);
        int lv = m_playerData.shopLevels[i];
        int cost = m_playerData.getUpgradeCost(item);

        // 获取描述键
        TextKey descKeys[] = {TextKey::Shop_Desc_StartGold, TextKey::Shop_Desc_Discount,
                              TextKey::Shop_Desc_Damage, TextKey::Shop_Desc_Range,
                              TextKey::Shop_Desc_Lives, TextKey::Shop_Desc_FireRate};

        // 名称键
        TextKey nameKeys[] = {TextKey::Shop_StartGold, TextKey::Shop_Discount,
                              TextKey::Shop_Damage, TextKey::Shop_Range,
                              TextKey::Shop_Lives, TextKey::Shop_FireRate};

        sb.nameText.setString(LangManager::get(nameKeys[i]));
        sb.levelText.setString(std::wstring(LangManager::get(descKeys[i])) +
                               L"  [Lv" + std::to_wstring(lv) + L"/" + std::to_wstring(SHOP_MAX_LEVEL) + L"]");

        if (lv >= SHOP_MAX_LEVEL)
        {
            sb.costText.setString(LangManager::get(TextKey::Shop_Max));
            sb.buyBtn.setFillColor(sf::Color(60, 60, 60));
            sb.buyLabel.setString(LangManager::get(TextKey::Shop_Max));
        }
        else
        {
            sb.costText.setString(std::to_wstring(cost) + L" " +
                                  std::wstring(LangManager::get(TextKey::CharSelect_Gold)));
            bool canBuy = m_playerData.totalGold >= cost;
            sb.buyBtn.setFillColor(canBuy ? sf::Color(40, 100, 40) : sf::Color(60, 40, 40));
            sb.buyLabel.setString(LangManager::get(TextKey::Shop_Buy));
        }
    }
}

void Game::renderShop()
{
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT + 100));
    bg.setFillColor(sf::Color(15, 15, 30));
    m_window.draw(bg);

    sf::FloatRect tb = m_shopTitleText.getLocalBounds();
    m_shopTitleText.setOrigin(tb.width / 2, tb.height / 2);
    m_shopTitleText.setPosition(WINDOW_WIDTH / 2.0f, 80);
    m_window.draw(m_shopTitleText);

    sf::FloatRect gb = m_shopGoldText.getLocalBounds();
    m_shopGoldText.setOrigin(gb.width / 2, gb.height / 2);
    m_shopGoldText.setPosition(WINDOW_WIDTH / 2.0f, 130);
    m_window.draw(m_shopGoldText);

    for (auto &sb : m_shopButtons)
    {
        m_window.draw(sb.bg);

        sb.nameText.setPosition(sb.bg.getPosition().x + 10, sb.bg.getPosition().y + 4);
        m_window.draw(sb.nameText);

        sb.levelText.setPosition(sb.bg.getPosition().x + 10, sb.bg.getPosition().y + 28);
        m_window.draw(sb.levelText);

        sb.costText.setPosition(sb.bg.getPosition().x + 400, sb.bg.getPosition().y + 16);
        m_window.draw(sb.costText);

        sb.buyBtn.setPosition(sb.bg.getPosition().x + 490, sb.bg.getPosition().y + 11);
        m_window.draw(sb.buyBtn);

        sf::FloatRect bl = sb.buyLabel.getLocalBounds();
        sb.buyLabel.setOrigin(bl.width / 2, bl.height / 2);
        sb.buyLabel.setPosition(sb.buyBtn.getPosition().x + 50, sb.buyBtn.getPosition().y + 17);
        m_window.draw(sb.buyLabel);
    }

    sf::FloatRect hb = m_shopBackHint.getLocalBounds();
    m_shopBackHint.setOrigin(hb.width / 2, hb.height / 2);
    m_shopBackHint.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT + 80);
    m_window.draw(m_shopBackHint);
}

void Game::processShopEvents(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float mx = worldPos.x, my = worldPos.y;
        for (auto &sb : m_shopButtons)
        {
            bool inside = sb.buyBtn.getGlobalBounds().contains(mx, my);
            sb.hovered = inside;
        }
        return;
    }
    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    float mx = worldPos.x, my = worldPos.y;

    for (int i = 0; i < SHOP_ITEM_COUNT; ++i)
    {
        auto &sb = m_shopButtons[i];
        if (sb.buyBtn.getGlobalBounds().contains(mx, my))
        {
            ShopItem item = static_cast<ShopItem>(i);
            if (m_playerData.canUpgrade(item) &&
                m_playerData.totalGold >= m_playerData.getUpgradeCost(item))
            {
                m_playerData.upgrade(item);
                m_playerData.save(PlayerData::makeSavePath(m_playerData.name));
                refreshShopTexts();
            }
            break;
        }
    }
}
