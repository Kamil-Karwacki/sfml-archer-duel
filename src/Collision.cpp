#include "Collision.h"
#include "Arrow.h"
#include "Destructible.h"
#include "Entity.h"
#include "Player.h"
#include "Powerup.h"
#include "SFML/System/Vector2.hpp"
#include <algorithm>
#include <cmath>
#include <future>
#include <mutex>
#include <ranges>
#include <thread>
#include <vector>

bool OBB::checkCollision(const Hitbox &other) const { return false; }

void OBB::draw(sf::RenderTarget &target, sf::Vector2f position,
               float rotation) const
{
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(halfSize.x * 2, halfSize.y * 2));
    rect.setOrigin(halfSize);
    rect.setPosition(position);
    rect.setRotation(sf::radians(rotation));
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineThickness(2.f);
    rect.setOutlineColor(sf::Color(250, 150, 100));
    target.draw(rect);
}

bool Circle::checkCollision(const Hitbox &other) const { return false; }

void Circle::draw(sf::RenderTarget &target, sf::Vector2f position,
                  float rotation) const
{
    sf::CircleShape circle(radius);
    circle.setPosition(position);
    circle.setFillColor(sf::Color::Red);

    target.draw(circle);
}

RigidBody::RigidBody(sf::Vector2f position, float _mass, float _inertia,
                     uint32_t layer, uint32_t mask,
                     std::unique_ptr<Hitbox> &&hitbox, sf::Vector2f velocity,
                     float rotation)
    : position(position), velocity(velocity), rotation(rotation), layer(layer),
      collisonMask(mask), hitbox(std::move(hitbox))
{
    if (_mass == 0 || _inertia == 0)
    {
        makeStatic();
    }
    else
    {
        mass = _mass;
        inverse_mass = 1.f / _mass;

        inertia = _inertia;
        inverse_inertia = 1.f / _inertia;
    }
}

void RigidBody::makeStatic()
{
    mass = 0;
    inverse_mass = 0;
    inertia = 0;
    inverse_inertia = 0;
}

void RigidBody::integrate(float dt)
{
    velocity += forces * dt * getInverseMass();
    angular_velocity += torques * dt * getInverseInertia();

    static constexpr float lineardDamping = 0.9f;
    static constexpr float angularDamping = 0.9f;

    sf::Vector2f damping = {pow(lineardDamping, dt), pow(angularDamping, dt)};

    velocity = velocity * damping.x;
    angular_velocity = angular_velocity * damping.y;

    position += velocity * dt;
    rotation += angular_velocity * dt;

    forces = sf::Vector2f(0, 0);
    torques = 0;
}

void RigidBody::addForce(sf::Vector2f force) { forces += force; }

bool shouldCollide(const uint32_t &layer1, const uint32_t mask1,
                   const uint32_t &layer2, const uint32_t mask2)
{
    return (mask1 & layer2) && (mask2 & layer1);
}

std::vector<sf::Vector2f> getOBBCorners(const RigidBody &rb, const OBB &obb)
{
    std::vector<sf::Vector2f> corners(4);

    sf::Vector2f localCorners[4] = {
        sf::Vector2f(-obb.halfSize.x, -obb.halfSize.y),
        sf::Vector2f(obb.halfSize.x, -obb.halfSize.y),
        sf::Vector2f(obb.halfSize.x, obb.halfSize.y),
        sf::Vector2f(-obb.halfSize.x, obb.halfSize.y)};

    float cos_r = cos(rb.rotation);
    float sin_r = sin(rb.rotation);

    for (int i = 0; i < 4; i++)
    {
        float rotX = localCorners[i].x * cos_r - localCorners[i].y * sin_r;
        float rotY = localCorners[i].x * sin_r + localCorners[i].y * cos_r;

        corners[i] = sf::Vector2f(rotX + rb.position.x, rotY + rb.position.y);
    }

    return corners;
}

std::vector<sf::Vector2f> getOBBAxes(const RigidBody &rb)
{
    std::vector<sf::Vector2f> axes(2);

    float cos_r = cos(rb.rotation);
    float sin_r = sin(rb.rotation);

    axes[0] = sf::Vector2f(cos_r, sin_r);
    axes[1] = sf::Vector2f(-sin_r, cos_r);

    return axes;
}

Contact calculateContactFromClipping(RigidBody &rb_A, const OBB &obb_A,
                                     RigidBody &rb_B, const OBB &obb_B,
                                     const sf::Vector2f &separatingAxis,
                                     float penetration)
{
    Contact contact;
    contact.A = &rb_A;
    contact.B = &rb_B;
    contact.normal = separatingAxis;
    contact.penetration = penetration;

    std::vector<sf::Vector2f> cornersA = getOBBCorners(rb_A, obb_A);
    std::vector<sf::Vector2f> cornersB = getOBBCorners(rb_B, obb_B);

    std::vector<sf::Vector2f> contactRegion = clipPolygon(cornersA, cornersB);

    if (contactRegion.empty())
    {
        contactRegion = clipPolygon(cornersB, cornersA);
    }

    if (!contactRegion.empty())
    {
        sf::Vector2f centroid(0, 0);
        for (const auto &point : contactRegion)
        {
            centroid += point;
        }
        centroid.x /= contactRegion.size();
        centroid.y /= contactRegion.size();
        contact.point = centroid;
    }
    else
    {
        contact.point = (rb_A.position + rb_B.position) * 0.5f;
    }

    return contact;
}

std::vector<sf::Vector2f> clipPolygon(const std::vector<sf::Vector2f> &subject,
                                      const std::vector<sf::Vector2f> &clipPoly)
{
    std::vector<sf::Vector2f> output = subject;

    for (size_t i = 0; i < clipPoly.size(); i++)
    {
        if (output.empty())
            break;

        std::vector<sf::Vector2f> input = output;
        output.clear();

        if (input.empty())
            continue;

        sf::Vector2f clipVertex1 = clipPoly[i];
        sf::Vector2f clipVertex2 = clipPoly[(i + 1) % clipPoly.size()];

        sf::Vector2f edge = clipVertex2 - clipVertex1;
        sf::Vector2f normal = sf::Vector2f(-edge.y, edge.x);

        if (input.empty())
            continue;

        sf::Vector2f s = input.back();

        for (size_t j = 0; j < input.size(); j++)
        {
            sf::Vector2f e = input[j];

            sf::Vector2f toE = e - clipVertex1;
            bool eInside = toE.dot(normal) >= 0;

            sf::Vector2f toS = s - clipVertex1;
            bool sInside = toS.dot(normal) >= 0;

            if (eInside)
            {
                if (!sInside)
                {
                    sf::Vector2f dir = e - s;
                    float denom = dir.dot(normal);
                    if (abs(denom) > 0.0001f)
                    {
                        float t = (clipVertex1 - s).dot(normal) / denom;
                        sf::Vector2f intersection = s + dir * t;
                        output.push_back(intersection);
                    }
                }
                output.push_back(e);
            }
            else if (sInside)
            {
                sf::Vector2f dir = e - s;
                float denom = dir.dot(normal);
                if (abs(denom) > 0.0001f)
                {
                    float t = (clipVertex1 - s).dot(normal) / denom;
                    sf::Vector2f intersection = s + dir * t;
                    output.push_back(intersection);
                }
            }

            s = e;
        }
    }

    return output;
}

Projection projectPolygon(const std::vector<sf::Vector2f> &vertices,
                          const sf::Vector2f &axis)
{
    Projection proj;
    proj.min = proj.max = vertices[0].dot(axis);

    for (size_t i = 1; i < vertices.size(); i++)
    {
        float dot = vertices[i].dot(axis);
        proj.min = std::min(proj.min, dot);
        proj.max = std::max(proj.max, dot);
    }

    return proj;
}

bool OBBvOBB(RigidBody &rb_A, OBB &obb_A, RigidBody &rb_B, OBB &obb_B,
             std::vector<Contact> &contacts, bool isTrigger)
{
    std::vector<sf::Vector2f> cornersA = getOBBCorners(rb_A, obb_A);
    std::vector<sf::Vector2f> cornersB = getOBBCorners(rb_B, obb_B);

    std::vector<sf::Vector2f> axesA = getOBBAxes(rb_A);
    std::vector<sf::Vector2f> axesB = getOBBAxes(rb_B);

    std::vector<sf::Vector2f> allAxes;
    allAxes.insert(allAxes.end(), axesA.begin(), axesA.end());
    allAxes.insert(allAxes.end(), axesB.begin(), axesB.end());

    float minPenetration = std::numeric_limits<float>::max();
    sf::Vector2f separatingAxis;
    bool foundSeparatingAxis = false;

    for (const auto &axis : allAxes)
    {
        Projection projA = projectPolygon(cornersA, axis);
        Projection projB = projectPolygon(cornersB, axis);

        if (projA.max < projB.min || projB.max < projA.min)
        {
            return false;
        }

        float overlap = std::min(projA.max - projB.min, projB.max - projA.min);

        if (overlap < minPenetration)
        {
            minPenetration = overlap;
            separatingAxis = axis;
            foundSeparatingAxis = true;
        }
    }

    if (!foundSeparatingAxis)
    {
        return false;
    }

    sf::Vector2f centerToCenter = rb_B.position - rb_A.position;
    if (centerToCenter.dot(separatingAxis) < 0)
    {
        separatingAxis = sf::Vector2f(-separatingAxis.x, -separatingAxis.y);
    }

    separatingAxis = separatingAxis.normalized();

    if (!isTrigger)
    {
        Contact contact = calculateContactFromClipping(
            rb_A, obb_A, rb_B, obb_B, separatingAxis, minPenetration);
        contacts.push_back(contact);
    }

    return true;
}

void CollisionResolver::processCollisionRange(
    const std::vector<std::unique_ptr<Entity>> &entities,
    std::vector<Contact> &contacts, WorkRange range)
{

    std::vector<Contact> local_contacts;

    auto i_range = std::views::iota(range.start_i, range.end_i + 1);

    for (int i : i_range)
    {
        int j_start = (i == range.start_i) ? range.start_j : i + 1;
        int j_end = (i == range.end_i) ? range.end_j
                                       : static_cast<int>(entities.size()) - 1;

        auto j_range = std::views::iota(j_start, j_end + 1) |
                       std::views::filter(
                           [&entities](int j)
                           { return j < static_cast<int>(entities.size()); });

        for (int j : j_range)
        {
            Entity *A = entities[i].get();
            Entity *B = entities[j].get();

            if (!shouldCollide(
                    A->getRigidbody()->layer, A->getRigidbody()->collisonMask,
                    B->getRigidbody()->layer, B->getRigidbody()->collisonMask))
                continue;

            if (auto *a_obb = dynamic_cast<OBB *>(A->getHitbox()))
            {
                if (auto *b_obb = dynamic_cast<OBB *>(B->getHitbox()))
                {
                    if (OBBvOBB(*A->getRigidbody(), *a_obb, *B->getRigidbody(),
                                *b_obb, local_contacts,
                                A->getHitbox()->isTrigger ||
                                    B->getHitbox()->isTrigger))
                    {
                        {
                            std::lock_guard<std::mutex> lock(
                                collision_state_mutex);
                            A->getRigidbody()->isColliding = true;
                            B->getRigidbody()->isColliding = true;
                        }
                        handleCollisionLogic(A, B);
                    }
                }
            }
        }
    }

    if (!local_contacts.empty())
    {
        std::lock_guard<std::mutex> lock(contacts_mutex);
        contacts.insert(contacts.end(),
                        std::make_move_iterator(local_contacts.begin()),
                        std::make_move_iterator(local_contacts.end()));
    }
}

void CollisionResolver::handleCollisionLogic(Entity *A, Entity *B)
{
    auto *arrowA = dynamic_cast<Arrow *>(A);
    auto *powerupB = dynamic_cast<PowerupEntity *>(B);
    if (arrowA && powerupB)
    {
        powerupB->onPickup(*arrowA->shooter);
        return;
    }

    auto *arrowB = dynamic_cast<Arrow *>(B);
    auto *powerupA = dynamic_cast<PowerupEntity *>(A);
    if (arrowB && powerupA)
    {
        powerupA->onPickup(*arrowB->shooter);
        return;
    }

    auto *destructibleA = dynamic_cast<Destructible *>(A);
    auto *destructibleB = dynamic_cast<Destructible *>(B);
    if (destructibleA && arrowB)
    {
        std::lock_guard<std::mutex> lock(collision_state_mutex);
        destructibleA->health -= arrowB->shooter->bow.arrowDamage;
        if (destructibleA->health <= 0)
            destructibleA->toBeDeleted = true;
        arrowB->toBeDeleted = true;
    }
    if (destructibleB && arrowA)
    {
        std::lock_guard<std::mutex> lock(collision_state_mutex);
        destructibleB->health -= arrowA->shooter->bow.arrowDamage;
        if (destructibleB->health <= 0)
            destructibleB->toBeDeleted = true;
        arrowA->toBeDeleted = true;
    }

    auto *playerA = dynamic_cast<Player *>(A);
    auto *playerB = dynamic_cast<Player *>(B);
    if (playerA && arrowB &&
        arrowB->getRigidbody()->velocity.dot(
            (playerA->getRigidbody()->position -
             arrowB->getRigidbody()->position)
                .normalized()) > 4.f)
    {
        std::lock_guard<std::mutex> lock(collision_state_mutex);
        playerA->health -= arrowB->shooter->bow.arrowDamage;
        arrowB->toBeDeleted = true;
    }
    if (playerB && arrowA &&
        arrowA->getRigidbody()->velocity.dot(
            (playerB->getRigidbody()->position -
             arrowA->getRigidbody()->position)
                .normalized()) > 4.f)
    {
        std::lock_guard<std::mutex> lock(collision_state_mutex);
        playerB->health -= arrowA->shooter->bow.arrowDamage;
        arrowA->toBeDeleted = true;
    }
}

std::vector<CollisionResolver::WorkRange>
CollisionResolver::calculateWorkRanges(int num_entities, int num_threads)
{
    std::vector<WorkRange> ranges;

    long total_pairs = (long)num_entities * (num_entities - 1) / 2;
    long pairs_per_thread = total_pairs / num_threads;

    long current_pair = 0;

    for (int thread = 0; thread < num_threads; ++thread)
    {
        WorkRange range;
        long start_pair = current_pair;
        long end_pair = (thread == num_threads - 1)
                            ? total_pairs - 1
                            : current_pair + pairs_per_thread - 1;

        auto pairToCoords = [](long pair_idx, int n) -> std::pair<int, int>
        {
            int i = 0;
            while (pair_idx >= n - i - 1)
            {
                pair_idx -= (n - i - 1);
                i++;
            }
            int j = i + 1 + pair_idx;
            return {i, j};
        };

        auto start_coords = pairToCoords(start_pair, num_entities);
        auto end_coords = pairToCoords(end_pair, num_entities);

        range.start_i = start_coords.first;
        range.start_j = start_coords.second;
        range.end_i = end_coords.first;
        range.end_j = end_coords.second;

        ranges.push_back(range);
        current_pair = end_pair + 1;
    }

    return ranges;
}

void CollisionResolver::detectCollisionsMultithreaded(
    const std::vector<std::unique_ptr<Entity>> &entities,
    std::vector<Contact> &contacts)
{

    int num_entities = entities.size();

    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
        num_threads = 2;

    std::vector<WorkRange> work_ranges =
        calculateWorkRanges(num_entities, num_threads);
    std::vector<std::future<void>> futures;

    for (const auto &range : work_ranges)
    {
        futures.push_back(
            std::async(std::launch::async, [&entities, &contacts, range]()
                       { processCollisionRange(entities, contacts, range); }));
    }

    for (auto &future : futures)
    {
        future.wait();
    }
}

void CollisionResolver::resolveContact(Contact &contact)
{
    RigidBody *bodyA = contact.A;
    RigidBody *bodyB = contact.B;

    if (bodyA->getInverseMass() == 0 && bodyB->getInverseMass() == 0)
        return;

    positionalCorrection(contact);

    resolveCollision(contact);
}

void CollisionResolver::positionalCorrection(Contact &contact)
{
    const float percent = 0.8f;
    const float slop = 0.01f;

    float totalInverseMass = 0.0f;
    if (!(contact.A->getInverseMass() == 0))
        totalInverseMass += 1.0f / contact.A->getMass();
    if (!(contact.B->getInverseMass() == 0))
        totalInverseMass += 1.0f / contact.B->getMass();

    if (totalInverseMass <= 0)
        return;

    sf::Vector2f correction =
        contact.normal *
        (std::max(contact.penetration - slop, 0.0f) / totalInverseMass) *
        percent;

    if (!(contact.A->getInverseMass() == 0))
        contact.A->position -= correction * (1.0f / contact.A->getMass());
    if (!(contact.B->getInverseMass() == 0))
        contact.B->position += correction * (1.0f / contact.B->getMass());
}

void CollisionResolver::resolveCollision(Contact &contact)
{
    RigidBody *bodyA = contact.A;
    RigidBody *bodyB = contact.B;

    sf::Vector2f rA = contact.point - bodyA->position;
    sf::Vector2f rB = contact.point - bodyB->position;

    sf::Vector2f relativeVelocity = getRelativeVelocity(bodyA, bodyB, rA, rB);

    float separatingVelocity = relativeVelocity.dot(contact.normal);

    if (separatingVelocity > 0)
        return;

    float restitution = std::min(bodyA->restitution, bodyB->restitution);

    float newSeparatingVelocity = -separatingVelocity * restitution;

    float deltaVelocity = newSeparatingVelocity - separatingVelocity;

    float totalInverseMass =
        calculateTotalInverseMass(bodyA, bodyB, rA, rB, contact.normal);

    if (totalInverseMass <= 0)
        return;

    float impulse = deltaVelocity / totalInverseMass;
    sf::Vector2f impulseVector = contact.normal * impulse;

    if (!(bodyA->getInverseMass() == 0))
    {
        bodyA->velocity -= impulseVector * (1.0f / bodyA->getMass());
        bodyA->angular_velocity -=
            rA.cross(impulseVector) / bodyA->getInertia();
    }
    if (!(bodyB->getInverseMass() == 0))
    {
        bodyB->velocity += impulseVector * (1.0f / bodyB->getMass());
        bodyB->angular_velocity +=
            rB.cross(impulseVector) / bodyB->getInertia();
    }

    resolveFriction(contact, impulse);
}

void CollisionResolver::resolveFriction(Contact &contact, float normalImpulse)
{
    RigidBody *bodyA = contact.A;
    RigidBody *bodyB = contact.B;

    sf::Vector2f rA = contact.point - bodyA->position;
    sf::Vector2f rB = contact.point - bodyB->position;

    sf::Vector2f relativeVelocity = getRelativeVelocity(bodyA, bodyB, rA, rB);

    sf::Vector2f tangent =
        relativeVelocity -
        contact.normal * relativeVelocity.dot(contact.normal);

    float tangentLength = tangent.length();
    if (tangentLength < 0.001f)
        return;

    tangent = tangent / tangentLength;

    float totalInverseMass =
        calculateTotalInverseMass(bodyA, bodyB, rA, rB, tangent);
    if (totalInverseMass <= 0)
        return;

    float frictionImpulse = -relativeVelocity.dot(tangent) / totalInverseMass;

    float friction = std::sqrt(bodyA->friction * bodyA->friction +
                               bodyB->friction * bodyB->friction);

    sf::Vector2f frictionVector;
    if (std::abs(frictionImpulse) < normalImpulse * friction)
    {
        frictionVector = tangent * frictionImpulse;
    }
    else
    {
        frictionVector = tangent * (-normalImpulse * friction);
    }

    if (!(bodyA->getInverseMass() == 0))
    {
        bodyA->velocity -= frictionVector * (1.0f / bodyA->getMass());
        bodyA->angular_velocity -=
            rA.cross(frictionVector) / bodyA->getInertia();
    }
    if (!(bodyB->getInverseMass() == 0))
    {
        bodyB->velocity += frictionVector * (1.0f / bodyB->getMass());
        bodyB->angular_velocity +=
            rB.cross(frictionVector) / bodyB->getInertia();
    }
}

sf::Vector2f CollisionResolver::getRelativeVelocity(RigidBody *bodyA,
                                                    RigidBody *bodyB,
                                                    const sf::Vector2f &rA,
                                                    const sf::Vector2f &rB)
{
    sf::Vector2f velA =
        bodyA->velocity + perpendicular(rA) * bodyA->angular_velocity;
    sf::Vector2f velB =
        bodyB->velocity + perpendicular(rB) * bodyB->angular_velocity;
    return velB - velA;
}

float CollisionResolver::calculateTotalInverseMass(RigidBody *bodyA,
                                                   RigidBody *bodyB,
                                                   const sf::Vector2f &rA,
                                                   const sf::Vector2f &rB,
                                                   const sf::Vector2f &normal)
{
    float totalInverseMass = 0.0f;

    if (!(bodyA->getInverseMass() == 0))
    {
        totalInverseMass += 1.0f / bodyA->getMass();
        float rACrossN = rA.cross(normal);
        totalInverseMass += (rACrossN * rACrossN) / bodyA->getInertia();
    }

    if (!(bodyB->getInverseMass() == 0))
    {
        totalInverseMass += 1.0f / bodyB->getMass();
        float rBCrossN = rB.cross(normal);
        totalInverseMass += (rBCrossN * rBCrossN) / bodyB->getInertia();
    }

    return totalInverseMass;
}
sf::Vector2f CollisionResolver::perpendicular(const sf::Vector2f &v)
{
    return sf::Vector2f(-v.y, v.x);
}
