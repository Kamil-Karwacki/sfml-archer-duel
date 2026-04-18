#include "Destructible.h"

Destructible::Destructible(sf::Vector2f position, sf::Vector2f size,
                           float health, std::string textureFile)
{
    auto hitbox = std::make_unique<OBB>(sf::Vector2f(0, 0), size, 0);
    drawable = std::make_unique<DrawableComp>(
        textureFile, sf::Vector2f(size.x * 5.f, size.y * 3.2f));
    drawable->sprite.setPosition(position);
    rb = std::make_unique<RigidBody>(
        position, 0, 0, RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_ENEMY | RigidBody::LAYER_ENEMY_ARROW |
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW,
        std::move(hitbox));
}
