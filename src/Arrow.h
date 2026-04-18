#pragma once
#include "Entity.h"
#include "Player.h"

class Arrow : public Entity
{
  public:
    float timer = 0.f;
    ;
    float expirationTime = 10.f;
    std::vector<std::unique_ptr<Entity>> &entities;
    Player *shooter;
    Arrow(sf::Vector2f position, std::vector<std::unique_ptr<Entity>> &entities,
          Player *shooter, uint32_t layer, uint32_t mask,
          std::string textureFile = "assets/dev_texture.png");
    void update(float dt) override;
};
