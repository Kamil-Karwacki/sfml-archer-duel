#include "MainMenuState.h"

#include "BuyingMenuState.h"
#include "CharacterCreationState.h"
#include "FileIO.h"
#include "GameManager.h"
#include "GameState.h"
#include <filesystem>
#include <iostream>
#include <vector>

MainMenuState::MainMenuState(GameManager *manager)
    : manager(manager),
      startNewGameBtn(
          Button(sf::Vector2f(200, 50),
                 (static_cast<sf::Vector2f>(manager->window.getSize()) / 2.f) +
                     sf::Vector2f(-100, 0),
                 "Start new game", font)),
      loadGameBtn(
          Button(sf::Vector2f(200, 50),
                 (static_cast<sf::Vector2f>(manager->window.getSize()) / 2.f) +
                     sf::Vector2f(-100, 75),
                 "Load game", font)),
      exitBtn(
          Button(sf::Vector2f(200, 50),
                 (static_cast<sf::Vector2f>(manager->window.getSize()) / 2.f) +
                     sf::Vector2f(-100, 150),
                 "Exit", font)),
      returnBtn(
          Button(sf::Vector2f(200, 50),
                 (static_cast<sf::Vector2f>(manager->window.getSize()) / 2.f) +
                     sf::Vector2f(-100, -300),
                 "Return", font))
{
    font = sf::Font("assets/CONSOLA.TTF");

    fs::path filepath = std::string("saves/");
    bool doesDirExist = fs::is_directory(filepath.parent_path());
    if (!doesDirExist)
    {
        fs::create_directory("saves/");
    }

    std::vector<std::string> filenames = listFilesInDirectory("saves/");
    for (size_t i = 0; i < filenames.size(); i++)
    {
        Button btn(sf::Vector2f(200, 50),
                   sf::Vector2f((manager->window.getSize().x / 3.0f) - 100,
                                150 + 60 * i),
                   filenames[i], font);
        btn.setCallback(
            [=, this]()
            {
                if (!loadingSaves)
                    return;
                loadData("saves/" + filenames[i], manager->playerName,
                         manager->playerMoney, manager->upgradeCost,
                         manager->slayedEnemies, manager->enemyInnacuracy);
                manager->setState(std::make_unique<BuyingMenuState>(manager));
            });
        saveGamesBtns.push_back(btn);
    }

    startNewGameBtn.setCallback(
        [=]()
        {
            if (loadingSaves)
                return;
            manager->setState(
                std::make_unique<CharacterCreationState>(manager));
        });

    loadGameBtn.setCallback(
        [=, this]()
        {
            if (loadingSaves)
                return;
            loadingSaves = true;
        });
    returnBtn.setCallback(
        [=, this]()
        {
            if (!loadingSaves)
                return;
            loadingSaves = false;
        });

    exitBtn.setCallback(
        [=]()
        {
            if (loadingSaves)
                return;
            if (manager)
                manager->window.close();
        });
}

void MainMenuState::handleEvent(GameManager &manager, sf::Event &event)
{
    startNewGameBtn.handleEvent(manager.window, event);
    loadGameBtn.handleEvent(manager.window, event);
    exitBtn.handleEvent(manager.window, event);
    returnBtn.handleEvent(manager.window, event);
    for (auto &btn : saveGamesBtns)
    {
        btn.handleEvent(manager.window, event);
    }
}

void MainMenuState::draw(GameManager &manager, sf::RenderWindow &window)
{
    if (loadingSaves)
    {
        returnBtn.render(window);
        for (auto &btn : saveGamesBtns)
        {
            btn.render(window);
        }
    }
    else
    {
        startNewGameBtn.render(window);
        loadGameBtn.render(window);
        exitBtn.render(window);
    }
}
