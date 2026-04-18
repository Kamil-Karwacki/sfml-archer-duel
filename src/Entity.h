#pragma once
#include "Collision.h"
#include "Render.h"
#include <memory>

class Entity
{
  protected:
    std::unique_ptr<RigidBody> rb;
    std::unique_ptr<DrawableComp> drawable;

  public:
    bool toBeDeleted = false;
    Entity() : rb() {}
    Entity(std::unique_ptr<RigidBody> &&rb,
           std::unique_ptr<DrawableComp> &&drawable = nullptr)
        : rb(std::move(rb)), drawable(std::move(drawable))
    {
    }

    virtual void update(float dt);

    virtual void draw(sf::RenderWindow &window);

    RigidBody *getRigidbody() { return rb.get(); }
    Hitbox *getHitbox() { return rb->getHitbox(); }
};

std::unique_ptr<Entity> makePlatform(sf::Vector2f position, sf::Vector2f size);
