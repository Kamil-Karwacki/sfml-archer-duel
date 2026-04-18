#include "Arrow.h"
#include <cmath>
#define M_PI 3.14159265f
Arrow::Arrow(sf::Vector2f position,
             std::vector<std::unique_ptr<Entity>> &entities, Player *shooter,
             uint32_t layer, uint32_t mask, std::string textureFile)
    : entities(entities), shooter(shooter)
{
    std::unique_ptr<Hitbox> hitbox =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(20, 6), 0);
    rb =
        std::make_unique<RigidBody>(position, 1, 1, layer, mask,
                                    std::unique_ptr<Hitbox>(std::move(hitbox)));
    drawable =
        std::make_unique<DrawableComp>(textureFile, sf::Vector2f(60, 18));
}

void Arrow::update(float dt)
{
    timer += dt;
    if (timer > expirationTime)
        toBeDeleted = true;

    rb->addForce(sf::Vector2f(0, 500));

    if (rb->velocity.x != 0 || rb->velocity.y != 0)
    {
        float targetAngle = atan2(rb->velocity.y, rb->velocity.x);
        float angleDiff = targetAngle - rb->rotation;
        while (angleDiff > M_PI)
            angleDiff -= 2 * M_PI;
        while (angleDiff < -M_PI)
            angleDiff += M_PI;

        rb->addTorque(angleDiff * rb->getInertia() * rb->velocity.length());
    }

    rb->angular_velocity *= 0.999f;

    rb->integrate(dt);
    drawable->sprite.setPosition(rb->position);
    drawable->sprite.setRotation(sf::radians(rb->rotation));
}
