#include "Player.h"
#include "Powerup.h"
#include <iostream>

Player::Player(float health, sf::Vector2f position,
               std::vector<std::unique_ptr<Entity>> *entities,
               std::string textureFile)
    : entities(entities), maxHealth(health), speed(3000)
{
    auto hitbox =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(40, 40), 0);
    drawable =
        std::make_unique<DrawableComp>(textureFile, sf::Vector2f(80, 80));
    drawable->sprite.setOrigin(dynamic_cast<OBB *>(hitbox.get())->halfSize);
    drawable->sprite.setPosition(position);
    rb = std::make_unique<RigidBody>(position, 5, 1, RigidBody::LAYER_PLAYER,
                                     RigidBody::LAYER_ENEMY |
                                         RigidBody::LAYER_ENVIRONMENT |
                                         RigidBody::LAYER_ENEMY_ARROW,
                                     std::move(hitbox));

    bow = Bow("Shortbow", 10.f, 0.7f, 135.f, 48.f,
              std::make_unique<DrawableComp>("assets/Shortbow.png",
                                             sf::Vector2f{48.f, 48.f}));

    auto hitbox2 =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(50, 15), 0);
    hitbox2->isTrigger = true;
    auto rb = std::make_unique<RigidBody>(
        sf::Vector2f(getRigidbody()->position), 1, 1,
        RigidBody::LAYER_ENVIRONMENT, RigidBody::LAYER_ENVIRONMENT,
        std::unique_ptr<Hitbox>(std::move(hitbox2)));
    auto groundEntity = std::make_unique<Entity>(
        std::unique_ptr<RigidBody>(std::move(rb)), nullptr);
    isGroundedEntity = groundEntity.get();

    entities->push_back(std::move(groundEntity));
}

void Player::addPowerup(std::unique_ptr<Powerup> powerup)
{
    powerup->apply(*this);
    activePowerups.push_back(std::move(powerup));
}

void Player::update(float dt)
{
    isGroundedEntity->getRigidbody()->position =
        rb->position + sf::Vector2f(0, 45);
    if (isGroundedEntity->getRigidbody()->isColliding)
        hasDoubleJumped = false;

    static constexpr float gravity = 650.0f;

    rb->addForce(sf::Vector2f(0, gravity * getRigidbody()->getMass()));

    move();

    rb->integrate(dt);
    rb->rotation = 0;
    if (drawable)
    {
        drawable->sprite.setPosition(rb->position);
        drawable->sprite.setRotation(sf::radians(rb->rotation));
    }

    for (auto it = activePowerups.begin(); it != activePowerups.end();)
    {
        (*it)->update(*this, dt);
        if ((*it)->isExpired())
        {
            it = activePowerups.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!canShoot)
        shootingTimer += dt;

    if (shootingTimer >= bow.reloadTime &&
        !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        canShoot = true;
        shootingTimer = 0;
    }
}

void Player::move()
{
    rb->addForce(sf::Vector2f(speed * input.x, 0));

    if (input.y == -1)
    {
        if (isGroundedEntity->getRigidbody()->isColliding)
            rb->addForce(sf::Vector2f(0, -11000 * rb->getMass()));
        if (!isGroundedEntity->getRigidbody()->isColliding && doubleJump &&
            !hasDoubleJumped && lastInput.y >= 0)
        {
            rb->addForce(sf::Vector2f(0, -400000 * rb->getMass()));
            hasDoubleJumped = true;
        }
    }

    rb->addForce({-rb->velocity.x * 5.f, 0});
    lastInput = input;
}

void Player::shoot() {}

void Player::draw(sf::RenderWindow &window)
{
    if (drawable)
        drawable->draw(window);

    if (bow.drawable)
        bow.drawable->draw(window);
}
