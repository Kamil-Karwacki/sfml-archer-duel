#pragma once
#include "Bow.h"
#include "Entity.h"

class Powerup;

class Player : public Entity
{
  protected:
    std::vector<std::unique_ptr<Entity>> *entities = nullptr;
    bool hasDoubleJumped = false;
    std::vector<std::unique_ptr<Powerup>> activePowerups;

  public:
    Entity *isGroundedEntity;
    float maxHealth;
    float health = maxHealth;
    float speed;
    bool doubleJump = false;
    bool doubleArrow = false;
    void update(float dt) override;
    sf::Vector2i input{0, 0};
    sf::Vector2i lastInput{0, 0}; // input from last frame
    Bow bow;
    bool canShoot = true;
    float shootingTimer = 0;

    Player() = default;
    Player(float health, sf::Vector2f position,
           std::vector<std::unique_ptr<Entity>> *entities,
           std::string textureFile = "assets/dev_texture.png");

    void move();
    void shoot();
    void draw(sf::RenderWindow &window) override;
    void setInput(sf::Vector2i _input) { input = _input; }
    void addPowerup(std::unique_ptr<Powerup> powerup);
};
