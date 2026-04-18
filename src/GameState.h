#pragma once
#include "AIEnemy.h"
#include <SFML/Graphics.hpp>

class GameManager;

class GameState
{
  public:
    virtual ~GameState() = default;

    virtual void handleEvent(GameManager &manager, sf::Event &event) = 0;
    virtual void update(GameManager &manager, float dt) = 0;
    virtual void draw(GameManager &manager, sf::RenderWindow &window) = 0;
};
