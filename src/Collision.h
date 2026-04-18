#pragma once
#include <SFML/Graphics.hpp>
#include <mutex>

class Entity;

class Hitbox
{
  public:
    bool isTrigger = false;
    virtual ~Hitbox() {}

    virtual bool checkCollision(const Hitbox &other) const = 0;
    virtual void draw(sf::RenderTarget &target, sf::Vector2f position,
                      float rotation) const = 0;
};

class OBB : public Hitbox
{
  public:
    sf::Vector2f position;
    sf::Vector2f halfSize;
    OBB() : position(sf::Vector2f(0, 0)), halfSize(sf::Vector2f(32, 32)) {}
    OBB(float posX, float posY, float halfSizeX, float halfSizeY,
        float rotation)
        : position(sf::Vector2f(posX, posY)),
          halfSize(sf::Vector2f(halfSizeX, halfSizeY))
    {
    }
    OBB(sf::Vector2f position, sf::Vector2f halfSize, float rotation)
        : position(position), halfSize(halfSize)
    {
    }

    bool checkCollision(const Hitbox &other) const override;
    void draw(sf::RenderTarget &target, sf::Vector2f position,
              float rotation) const override;
};

class Circle : public Hitbox
{
  private:
    sf::Vector2f position;
    float radius;

  public:
    Circle(sf::Vector2f position, float radius)
        : position(position), radius(radius)
    {
    }

    bool checkCollision(const Hitbox &other) const override;
    void draw(sf::RenderTarget &target, sf::Vector2f position,
              float rotation = 0) const override;
};

class RigidBody
{
  private:
    float mass;
    float inverse_mass;
    float inertia;
    float inverse_inertia;

  protected:
    std::unique_ptr<Hitbox> hitbox;

  public:
    sf::Vector2f position;
    sf::Vector2f velocity = {0, 0};
    sf::Vector2f forces = {0, 0};

    float angular_velocity = 0;
    float rotation;
    float torques = 0;
    float restitution = 0.3f;
    float friction = 0.2f;

    uint32_t layer = 0;
    uint32_t collisonMask = 0;

    bool isColliding = false;

    enum Layer : uint32_t
    {
        LAYER_PLAYER = 1 << 0,
        LAYER_ENEMY = 1 << 1,
        LAYER_PLAYER_ARROW = 1 << 2,
        LAYER_ENEMY_ARROW = 1 << 3,
        LAYER_ENVIRONMENT = 1 << 4,
    };

    RigidBody()
        : position(sf::Vector2f(0, 0)), velocity(sf::Vector2f(0, 0)), mass(1),
          inverse_mass(1), inertia(1), inverse_inertia(1), rotation(0)
    {
    }
    RigidBody(sf::Vector2f position, float _mass, float _inertia,
              uint32_t layer, uint32_t mask, std::unique_ptr<Hitbox> &&hitbox,
              sf::Vector2f velocity = sf::Vector2f(0, 0), float rotation = 0);

    void makeStatic();

    void integrate(float dt);
    void addForce(sf::Vector2f force);
    void addTorque(float torque) { torques += torque; }
    float getInverseMass() { return inverse_mass; }
    float getMass() { return mass; }
    float getInertia() { return inertia; }
    float getInverseInertia() { return inverse_inertia; }
    Hitbox *getHitbox() { return hitbox.get(); }
    void applyImpulse(const sf::Vector2f &impulse)
    {
        velocity += impulse / mass;
    }
};

struct Contact
{
    RigidBody *A;
    RigidBody *B;
    sf::Vector2f normal;
    sf::Vector2f point;
    float penetration;
    bool isResting;
};

bool shouldCollide(const uint32_t &layer1, const uint32_t mask1,
                   const uint32_t &layer2, const uint32_t mask2);
std::vector<sf::Vector2f> getOBBCorners(const RigidBody &rb, const OBB &obb);

std::vector<sf::Vector2f> getOBBAxes(const RigidBody &rb);

struct Projection
{
    float min, max;
};

Projection projectPolygon(const std::vector<sf::Vector2f> &vertices,
                          const sf::Vector2f &axis);

std::vector<sf::Vector2f>
clipPolygon(const std::vector<sf::Vector2f> &subject,
            const std::vector<sf::Vector2f> &clipPoly);

Contact calculateContactFromClipping(RigidBody &rb_A, const OBB &obb_A,
                                     RigidBody &rb_B, const OBB &obb_B,
                                     const sf::Vector2f &separatingAxis,
                                     float penetration);

bool OBBvOBB(RigidBody &rb_A, OBB &obb_A, RigidBody &rb_B, OBB &obb_B,
             std::vector<Contact> &contacts, bool isTrigger);

class CollisionResolver
{
  public:
    static void resolveContact(Contact &contact);

    static void detectCollisionsMultithreaded(
        const std::vector<std::unique_ptr<Entity>> &entities,
        std::vector<Contact> &contacts);

  private:
    static void positionalCorrection(Contact &contact);

    static void resolveCollision(Contact &contact);

    static void resolveFriction(Contact &contact, float normalImpulse);

    static sf::Vector2f getRelativeVelocity(RigidBody *bodyA, RigidBody *bodyB,
                                            const sf::Vector2f &rA,
                                            const sf::Vector2f &rB);

    static float calculateTotalInverseMass(RigidBody *bodyA, RigidBody *bodyB,
                                           const sf::Vector2f &rA,
                                           const sf::Vector2f &rB,
                                           const sf::Vector2f &normal);
    static sf::Vector2f perpendicular(const sf::Vector2f &v);

    static std::mutex contacts_mutex;
    static std::mutex collision_state_mutex;

    struct WorkRange
    {
        int start_i;
        int end_i;
        int start_j;
        int end_j;
    };

    static void
    processCollisionRange(const std::vector<std::unique_ptr<Entity>> &entities,
                          std::vector<Contact> &contacts, WorkRange range);

    static void handleCollisionLogic(Entity *A, Entity *B);

    static std::vector<WorkRange> calculateWorkRanges(int num_entities,
                                                      int num_threads);
};
