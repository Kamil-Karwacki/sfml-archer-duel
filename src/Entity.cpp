#include "Entity.h"
#include "SFML/System/Vector2.hpp"

void Entity::update(float dt)
{
    if (drawable)
    {
        drawable->sprite.setPosition(rb->position);
        drawable->sprite.setRotation(sf::radians(rb->rotation));
    }
}

void Entity::draw(sf::RenderWindow &window)
{
    if (drawable)
        drawable->draw(window);
}

std::unique_ptr<Entity> makePlatform(sf::Vector2f position, sf::Vector2f size)
{
    auto platformHitbox = std::make_unique<OBB>(
        sf::Vector2f(0, 0), sf::Vector2f(size.x, size.y), 0);

    auto platformRb = std::make_unique<RigidBody>(
        position, 0, 0, RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_ENEMY | RigidBody::LAYER_ENEMY_ARROW |
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW |
            RigidBody::LAYER_ENVIRONMENT,
        std::unique_ptr<Hitbox>(std::move(platformHitbox)));
    auto platformDrawable = std::make_unique<DrawableComp>(
        "assets/platform.png", sf::Vector2f(size.x * 2.5f, size.y * 5));
    auto platform = std::make_unique<Entity>(
        std::unique_ptr<RigidBody>(std::move(platformRb)),
        std::unique_ptr<DrawableComp>(std::move(platformDrawable)));
    return platform;
}
