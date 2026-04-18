#pragma once
#include "Entity.h"

class Destructible : public Entity
{
  public:
    float maxHealth = 150;
    float health = maxHealth;

    Destructible(sf::Vector2f position, sf::Vector2f size, float health,
                 std::string textureFile);
};
