#include "Powerup.h"
#include "Player.h"
#include <cmath>

PowerupEntity::PowerupEntity(std::unique_ptr<Powerup> _effect,
                             sf::Vector2f position, std::string textureFile)
    : effect(std::move(_effect))
{
    auto hitbox =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(40, 40), 0);
    hitbox->isTrigger = true;
    drawable =
        std::make_unique<DrawableComp>(textureFile, sf::Vector2f(80, 80));
    drawable->sprite.setOrigin(dynamic_cast<OBB *>(hitbox.get())->halfSize);
    drawable->sprite.setPosition(position);
    rb = std::make_unique<RigidBody>(
        position, 15, 15, RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_PLAYER_ARROW | RigidBody::LAYER_ENVIRONMENT |
            RigidBody::LAYER_ENEMY_ARROW,
        std::move(hitbox));
    yLevel = rb->position.y;
}

void PowerupEntity::onPickup(Player &player)
{
    if (!effect)
        return;

    player.addPowerup(std::move(effect));
    this->toBeDeleted = true;
}

void PowerupEntity::update(float dt)
{
    Entity::update(dt);
    accumulatedTime += dt;
    int dir = speed;

    if (rb->position.x < 100)
        right = true;
    if (rb->position.x > 1180)
        right = false;

    if (!right)
        dir *= -1;

    rb->position.x += dir * dt;

    rb->position.y = yLevel + sin(accumulatedTime) * 20;
    rb->integrate(dt);
}
void SpeedPowerup::apply(Player &player)
{
    if (!applied)
    {
        player.speed *= speedMultiplier;
        applied = true;
    }
}

void SpeedPowerup::update(Player &player, float dt)
{
    timer += dt;
    if (timer >= duration)
    {
        expire(player);
    }
}

void SpeedPowerup::expire(Player &player)
{
    if (applied)
    {
        player.speed /= speedMultiplier;
        applied = false;
    }
}

bool SpeedPowerup::isExpired() const { return timer >= duration; }

void DoubleJumpPowerup::apply(Player &player)
{
    if (!applied)
    {
        player.doubleJump = true;
        applied = true;
    }
}

void DoubleJumpPowerup::update(Player &player, float dt)
{
    timer += dt;
    if (timer >= duration)
    {
        expire(player);
    }
}

void DoubleJumpPowerup::expire(Player &player)
{
    if (applied)
    {
        player.doubleJump = false;
        applied = false;
    }
}

bool DoubleJumpPowerup::isExpired() const { return timer >= duration; }
