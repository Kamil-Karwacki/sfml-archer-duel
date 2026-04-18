#include "GameManager.h"
#include "MainMenuState.h"
#include <chrono>

void GameManager::run()
{
    window = sf::RenderWindow(sf::VideoMode({1280, 768}), "PK4 projekt");
    setState(std::make_unique<MainMenuState>(this));

    this->loop();
}

void GameManager::loop()
{
    auto previous_time = std::chrono::high_resolution_clock::now();
    while (window.isOpen())
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(
                        current_time - previous_time)
                        .count();
        previous_time = current_time;

        if (nextState)
        {
            currentState = std::move(nextState);
            nextState = nullptr;
        }
        while (std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (currentState)
                currentState->handleEvent(*this, *event);
        }
        window.clear({100, 100, 100});

        currentState->update(*this, deltaTime);
        update();

        drawLoop();
    }
}

void GameManager::update() {}

void GameManager::drawLoop()
{
    currentState->draw(*this, window);
    window.display();
}

void GameManager::setState(std::unique_ptr<GameState> newState)
{
    nextState = std::move(newState);
}
