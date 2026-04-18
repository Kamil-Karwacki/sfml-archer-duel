#include "BuyingMenuState.h"
#include "ArenaState.h"
#include "GameManager.h"

#include "FileIO.h"
#include "GameManager.h"
#include "GameState.h"
BuyingMenuState::BuyingMenuState(GameManager *manager)
    : manager(manager),
      upgradeBowButton(Button(sf::Vector2f(220, 50), sf::Vector2f(600, 300),
                              "Upgrade your bow", font)),
      goFightButton(Button(sf::Vector2f(220, 50), sf::Vector2f(600, 400),
                           "Go and Fight!", font)),
      saveAndExitButton(Button(sf::Vector2f(220, 50), sf::Vector2f(600, 500),
                               "Save and exit", font)),
      backgroundTexture("assets/buyingBackground.png"),
      backgroundSprite(backgroundTexture)
{
    font = sf::Font("assets/CONSOLA.TTF");

    saveData("saves/" + manager->playerName + "_save.txt", manager->playerName,
             manager->playerMoney, manager->upgradeCost, manager->slayedEnemies,
             manager->enemyInnacuracy);
    upgradeBowButton.setCallback(
        [=]()
        {
            if (manager->playerMoney >= manager->upgradeCost)
            {
                manager->playerMoney -= manager->upgradeCost;
                manager->upgradeCost *= 2;
            }
        });

    goFightButton.setCallback(
        [=]() { manager->setState(std::make_unique<ArenaState>(manager)); });

    saveAndExitButton.setCallback(
        [=]()
        {
            saveData("saves/" + manager->playerName + "_save.txt",
                     manager->playerName, manager->playerMoney,
                     manager->upgradeCost, manager->slayedEnemies,
                     manager->enemyInnacuracy);
            manager->window.close();
        });
}

void BuyingMenuState::handleEvent(GameManager &manager, sf::Event &event)
{
    upgradeBowButton.handleEvent(manager.window, event);
    goFightButton.handleEvent(manager.window, event);
    saveAndExitButton.handleEvent(manager.window, event);
}

void BuyingMenuState::draw(GameManager &, sf::RenderWindow &window)
{
    font = sf::Font("assets/CONSOLA.TTF");
    window.draw(backgroundSprite);
    upgradeBowButton.render(window);
    goFightButton.render(window);
    saveAndExitButton.render(window);
    sf::Text enemiesText(font, "Slayed enemies: " +
                                   std::to_string(manager->slayedEnemies));
    sf::Text upgradeText(font, "Upgrade cost: " +
                                   std::to_string(manager->upgradeCost));
    sf::Text moneyText(font,
                       "Player money: " + std::to_string(manager->playerMoney));
    enemiesText.setPosition({900, 200});
    upgradeText.setPosition({900, 250});
    moneyText.setPosition({900, 300});

    window.draw(enemiesText);
    window.draw(upgradeText);
    window.draw(moneyText);
}
