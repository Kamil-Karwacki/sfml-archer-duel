#include "AIEnemy.h"
#include "Arrow.h"
#include "GameManager.h"
#include "Powerup.h"
#include <cmath>

AIEnemy::AIEnemy(float _health, sf::Vector2f position,
                 std::vector<std::unique_ptr<Entity>> *_entities,
                 Player *_player, GameManager *manager, std::string textureFile)
    : manager(manager)
{
    maxHealth = health = _health;
    player = _player;
    speed = 300;
    entities = _entities;
    auto hitbox =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(40, 40), 0);
    drawable =
        std::make_unique<DrawableComp>(textureFile, sf::Vector2f(80, 80));
    drawable->sprite.setOrigin(dynamic_cast<OBB *>(hitbox.get())->halfSize);
    drawable->sprite.setPosition(position);
    rb = std::make_unique<RigidBody>(position, 5, 1, RigidBody::LAYER_ENEMY,
                                     RigidBody::LAYER_PLAYER |
                                         RigidBody::LAYER_ENVIRONMENT |
                                         RigidBody::LAYER_PLAYER_ARROW,
                                     std::move(hitbox));

    bow = Bow("Shortbow", 10.f, 1.5f, 135.f, 48.f,
              std::make_unique<DrawableComp>("assets/Shortbow.png",
                                             sf::Vector2f{48.f, 48.f}));

    auto hitbox2 =
        std::make_unique<OBB>(sf::Vector2f(0, 0), sf::Vector2f(40, 10), 0);
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

void AIEnemy::update(float dt)
{
    isGroundedEntity->getRigidbody()->position =
        rb->position + sf::Vector2f(0, 45);
    if (isGroundedEntity->getRigidbody()->isColliding)
        hasDoubleJumped = false;

    rb->addForce(sf::Vector2f(0, 500 * getRigidbody()->getMass()));
    move();

    rb->integrate(dt);
    rb->rotation = 0;
    if (drawable)
    {
        drawable->sprite.setPosition(rb->position);
        drawable->sprite.setRotation(sf::radians(rb->rotation));
    }

    if (bow.drawable)
    {
        bow.drawable->sprite.setPosition(rb->position);
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

    if (canShoot)
    {
        tryShootAtPlayer(*player);
    }

    if (!canShoot)
        shootingTimer += dt;

    if (shootingTimer >= bow.reloadTime)
    {
        canShoot = true;
        shootingTimer = 0;
    }

    if (player->getRigidbody()->position.x < rb->position.x)
    {
        input.x = -1;
    }
    else if (player->getRigidbody()->position.x > rb->position.x)
    {
        input.x = 1;
    }

    sf::Vector2f dir = rb->position - player->getRigidbody()->position;
    if (bow.drawable)
    {
        bow.drawable->sprite.setRotation(
            sf::radians(std::atan2(-dir.y, -dir.x)) +
            sf::degrees(bow.degreeOffset));
        bow.drawable->sprite.setPosition(getRigidbody()->position -
                                         dir.normalized() * bow.distance);
    }
}

sf::Vector2f AIEnemy::calculateArrowVelocity(const sf::Vector2f &from,
                                             const sf::Vector2f &to,
                                             float gravity, float time)
{
    sf::Vector2f velocity;
    velocity.x = (to.x - from.x) / time;
    velocity.y = (to.y - from.y - 0.5f * gravity * time * time) / time;
    return velocity;
}

void AIEnemy::tryShootAtPlayer(Player &player)
{
    sf::Vector2f startPos = this->getRigidbody()->position;
    sf::Vector2f targetPos =
        (player.getRigidbody()->position * (1.f - manager->enemyInnacuracy));

    float gravity = 500.f;
    float bestTime = -1.f;
    sf::Vector2f bestVelocity;

    for (float t = 0.5f; t < 3.f; t += 0.1f)
    {
        sf::Vector2f vel =
            calculateArrowVelocity(startPos, targetPos, gravity, t);

        if (vel.length() < 1000)
        {
            bestVelocity = vel;
            bestTime = t;
            break;
        }
    }

    if (bestTime > 0)
    {
        sf::Vector2f direction = (targetPos - startPos).normalized();
        auto arrow = std::make_unique<Arrow>(
            rb->position + direction * bow.distance, *entities, &player,
            RigidBody::LAYER_ENEMY_ARROW,
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW |
                RigidBody::LAYER_ENVIRONMENT,
            "assets/arrow.png");
        arrow->shooter = this;
        float targetAngle = std::atan2(direction.y, direction.x);
        arrow->getRigidbody()->rotation = targetAngle;
        arrow->getRigidbody()->velocity = bestVelocity;
        entities->push_back(std::move(arrow));
        canShoot = false;
    }
}
