#include "ArenaState.h"

#include "Arrow.h"
#include "BuyingMenuState.h"
#include "Destructible.h"
#include "FileIO.h"
#include "GameManager.h"
#include "GameState.h"
#include "Powerup.h"
#include <iostream>
#include <random>
#include <vector>
std::mutex CollisionResolver::contacts_mutex;
std::mutex CollisionResolver::collision_state_mutex;

ArenaState::ArenaState(GameManager *_manager)
    : manager(_manager), backgroundTexture("assets/background.png"),
      backgroundSprite(backgroundTexture), font("assets/CONSOLA.TTF"),
      exitGameBtn(
          Button(sf::Vector2f(100, 50),
                 sf::Vector2f((_manager->window.getSize().x / 2.f) - 50.f,
                              2 * _manager->window.getSize().y / 3.f),
                 "Exit game", font)),
      continueBtn(
          Button(sf::Vector2f(100, 50),
                 sf::Vector2f((_manager->window.getSize().x / 2.f) - 50.f,
                              2 * _manager->window.getSize().y / 3.f),
                 "Continue", font))
{
    sf::Vector2f bounds =
        static_cast<sf::Vector2f>(backgroundSprite.getTextureRect().size);
    sf::Vector2f targetSize = {1280, 768};
    sf::Vector2f scale = {targetSize.x / bounds.x, targetSize.y / bounds.y};
    backgroundSprite.setColor({255, 255, 255, 130});
    backgroundSprite.setScale(scale);

    font = sf::Font("assets/CONSOLA.TTF");
    exitGameBtn.setCallback(
        [=, this]()
        {
            if (manager)
                manager->window.close();
        });

    continueBtn.setCallback(
        [=, this]()
        {
            if (manager)
                manager->setState(std::make_unique<BuyingMenuState>(manager));
        });

    // creating map //
    auto hitbox = std::make_unique<OBB>(
        sf::Vector2f(0, 0), sf::Vector2f(manager->window.getSize().x, 20), 0);
    auto rb = std::make_unique<RigidBody>(
        sf::Vector2f(manager->window.getSize().x / 2.0f,
                     manager->window.getSize().y - 30),
        0, 0, RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_ENEMY | RigidBody::LAYER_ENEMY_ARROW |
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW |
            RigidBody::LAYER_ENVIRONMENT,
        std::unique_ptr<Hitbox>(std::move(hitbox)));
    auto drawable = std::make_unique<DrawableComp>(
        "assets/ground.png", sf::Vector2f(manager->window.getSize().x, 60));
    auto groundPtr = std::make_unique<Entity>(
        std::unique_ptr<RigidBody>(std::move(rb)),
        std::unique_ptr<DrawableComp>(std::move(drawable)));

    auto hitboxL = std::make_unique<OBB>(
        sf::Vector2f(0, 0), sf::Vector2f(20, manager->window.getSize().y), 0);
    auto rbL = std::make_unique<RigidBody>(
        sf::Vector2f(0, manager->window.getSize().y), 0, 0,
        RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_ENEMY | RigidBody::LAYER_ENEMY_ARROW |
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW |
            RigidBody::LAYER_ENVIRONMENT,
        std::unique_ptr<Hitbox>(std::move(hitboxL)));
    auto leftWall =
        std::make_unique<Entity>(std::unique_ptr<RigidBody>(std::move(rbL)));

    auto hitboxR = std::make_unique<OBB>(
        sf::Vector2f(0, 0), sf::Vector2f(20, manager->window.getSize().y), 0);
    auto rbR = std::make_unique<RigidBody>(
        sf::Vector2f(manager->window.getSize().x - 10,
                     manager->window.getSize().y),
        0, 0, RigidBody::LAYER_ENVIRONMENT,
        RigidBody::LAYER_ENEMY | RigidBody::LAYER_ENEMY_ARROW |
            RigidBody::LAYER_PLAYER | RigidBody::LAYER_PLAYER_ARROW |
            RigidBody::LAYER_ENVIRONMENT,
        std::unique_ptr<Hitbox>(std::move(hitboxR)));
    auto rightWall =
        std::make_unique<Entity>(std::unique_ptr<RigidBody>(std::move(rbR)));

    generateFloatingPlatforms();

    entities.push_back(std::make_unique<PowerupEntity>(
        std::make_unique<SpeedPowerup>(), sf::Vector2f(100.f, 100.f),
        "assets/speedPowerup.png"));
    entities.push_back(std::make_unique<PowerupEntity>(
        std::make_unique<DoubleJumpPowerup>(), sf::Vector2f(700.f, 120.f),
        "assets/jumpPowerup.png"));

    auto playerPtr = std::make_unique<Player>(100, sf::Vector2f(100, 400),
                                              &entities, "assets/player.png");

    player = playerPtr.get();

    manager->enemyInnacuracy *= 0.9f;
    float enemyBaseHealth = 20;
    auto enemyPtr = std::make_unique<AIEnemy>(
        enemyBaseHealth + (enemyBaseHealth * 0.5f * manager->slayedEnemies),
        sf::Vector2f(600, 600), &entities, player, manager, "assets/enemy.png");
    enemy = enemyPtr.get();
    entities.push_back(std::move(playerPtr));
    entities.push_back(std::move(groundPtr));
    entities.push_back(std::move(leftWall));
    entities.push_back(std::move(rightWall));
    entities.push_back(std::move(enemyPtr));
}

void ArenaState::handleEvent(GameManager &manager, sf::Event &event)
{
    if (player->health <= 0)
        exitGameBtn.handleEvent(manager.window, event);
    if (enemy->health <= 0)
        continueBtn.handleEvent(manager.window, event);
}

void ArenaState::draw(GameManager &manager, sf::RenderWindow &window)
{
    if (hasEnded)
        return;
    window.draw(backgroundSprite);
    for (size_t i = 0; i < entities.size(); i++)
    {
        entities[i]->draw(window);

        if (Destructible *destr =
                dynamic_cast<Destructible *>(entities[i].get()))
        {
            sf::RectangleShape barOutline;
            sf::RectangleShape barInside;
            sf::Vector2f size = {100, 10};
            sf::Vector2f offset = {0, -20};
            barInside.setSize(sf::Vector2f(
                (size.x * destr->health / destr->maxHealth), size.y));
            barInside.setOrigin(size / 2.f);

            barOutline.setSize(size);
            barOutline.setOrigin(size / 2.f);
            if (OBB *obb = dynamic_cast<OBB *>(destr->getHitbox()))
            {
                barInside.setPosition(entities[i]->getRigidbody()->position -
                                      sf::Vector2f(0, obb->halfSize.y) +
                                      offset);
                barOutline.setPosition(entities[i]->getRigidbody()->position -
                                       sf::Vector2f(0, obb->halfSize.y) +
                                       offset);
            }
            barOutline.setFillColor(sf::Color::Transparent);
            barOutline.setOutlineThickness(2.f);
            barOutline.setOutlineColor(sf::Color(0, 0, 0));

            barInside.setFillColor(sf::Color::Red);

            window.draw(barInside);
            window.draw(barOutline);
        }

        if (Player *player = dynamic_cast<Player *>(entities[i].get()))
        {
            sf::RectangleShape barOutline;
            sf::RectangleShape barInside;
            sf::Vector2f size = {100, 10};
            sf::Vector2f offset = {0, -20};
            barInside.setSize(sf::Vector2f(
                (size.x * player->health / player->maxHealth), size.y));
            barInside.setOrigin(size / 2.f);

            barOutline.setSize(size);
            barOutline.setOrigin(size / 2.f);
            if (OBB *obb = dynamic_cast<OBB *>(player->getHitbox()))
            {
                barInside.setPosition(entities[i]->getRigidbody()->position -
                                      sf::Vector2f(0, obb->halfSize.y) +
                                      offset);
                barOutline.setPosition(entities[i]->getRigidbody()->position -
                                       sf::Vector2f(0, obb->halfSize.y) +
                                       offset);
            }
            barOutline.setFillColor(sf::Color::Transparent);
            barOutline.setOutlineThickness(2.f);
            barOutline.setOutlineColor(sf::Color(0, 0, 0));

            barInside.setFillColor(sf::Color::Red);

            window.draw(barInside);
            window.draw(barOutline);
        }
    }

    if (isAiming)
    {
        // drawing the line //
        shooting_dir =
            static_cast<sf::Vector2f>(shooting_end_pos - shooting_start_pos);
        if (shooting_dir.x == 0 && shooting_dir.y == 0)
        {
            return;
        }
        float length = std::sqrt(shooting_dir.x * shooting_dir.x +
                                 shooting_dir.y * shooting_dir.y);
        sf::RectangleShape line(sf::Vector2f(length, 5));

        line.setOrigin({0, 5.0f / 2});
        line.setPosition(static_cast<sf::Vector2f>(shooting_start_pos));
        line.setFillColor({255, 255, 255});
        line.setRotation(
            sf::radians(std::atan2(shooting_dir.y, shooting_dir.x)));

        window.draw(line);
        // drawing the bow //
        if (player->bow.drawable)
        {
            // sf::Vector2fshooting_dir =
            // static_cast<sf::Vector2f>(shooting_end_pos) -
            // player->getRigidbody()->position;
            player->bow.drawable->sprite.setRotation(
                sf::radians(std::atan2(-shooting_dir.y, -shooting_dir.x)) +
                sf::degrees(player->bow.degreeOffset));
            player->bow.drawable->sprite.setPosition(
                player->getRigidbody()->position -
                shooting_dir.normalized() * player->bow.distance);
        }
    }
    else
    {
        if (player->bow.drawable)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseDir = player->getRigidbody()->position -
                                    static_cast<sf::Vector2f>(mousePos);
            player->bow.drawable->sprite.setRotation(
                sf::radians(std::atan2(-mouseDir.y, -mouseDir.x)) +
                sf::degrees(player->bow.degreeOffset));
            player->bow.drawable->sprite.setPosition(
                player->getRigidbody()->position -
                mouseDir.normalized() * player->bow.distance);
        }
    }

    for (size_t i = 0; i < contacts.size(); ++i)
    {
        sf::CircleShape shape(5.f);
        shape.setFillColor(sf::Color(100, 250, 50));

        shape.setPosition(contacts[i].point);
        window.draw(shape);
    }
}

void ArenaState::processInput()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        input.x = 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        input.x = -1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
        input.y = -1;
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) &&
        !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        input.x = 0;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) &&
        !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        input.y = 0;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && player->canShoot)
    {
        if (!wasLMBpressed)
        {
            shooting_start_pos = sf::Mouse::getPosition(manager->window);
            isAiming = true;
        }
    }

    if (isAiming && player->canShoot)
    {
        shooting_end_pos = sf::Mouse::getPosition(manager->window);
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            isAiming = false;
    }

    if (wasLMBpressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
        player->canShoot)
    {
        shooting_end_pos = sf::Mouse::getPosition(manager->window);
        isShooting = true;
    }
    wasLMBpressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

void ArenaState::generateFloatingPlatforms()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> platformCountDist(4, 8);
    std::uniform_real_distribution<float> xDist(
        50.0f, manager->window.getSize().x - 150.0f);
    std::uniform_real_distribution<float> horizontalWiggle(-100.f, 100.f);
    std::uniform_real_distribution<float> yStepDist(80.f, 150.f);
    std::uniform_int_distribution<int> obstacleCountDist(1, 2);
    std::uniform_real_distribution<float> obstacleXDist(
        50.0f, manager->window.getSize().x - 100.0f);

    const float platformWidth = 60.f;
    const float platformHeight = 10.f;
    const float obstacleWidth = 16.f;
    const float obstacleHeight = 100.f;

    float currentY = manager->window.getSize().y - 150.f;
    float currentX = xDist(gen);

    std::vector<sf::FloatRect> placedRects;

    int numPlatforms = platformCountDist(gen);

    for (int i = 0; i < numPlatforms; ++i)
    {
        int retries = 10;
        bool placed = false;
        currentX = xDist(gen);
        while (retries-- > 0 && !placed)
        {
            sf::Vector2f candidatePos(currentX, currentY);
            sf::FloatRect candidateRect(
                candidatePos, sf::Vector2f(platformWidth, platformHeight));

            bool overlaps = false;
            for (const auto &rect : placedRects)
            {
                if (candidateRect.findIntersection(rect))
                {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps)
            {
                entities.push_back(makePlatform(
                    candidatePos, sf::Vector2f(platformWidth, platformHeight)));
                placedRects.push_back(candidateRect);
                placed = true;
            }
            else
            {
                currentX += horizontalWiggle(gen);
                currentX = std::clamp(currentX, 50.f,
                                      manager->window.getSize().x -
                                          platformWidth - 50.f);
            }
        }

        float yStep = yStepDist(gen);
        currentY -= yStep;
        if (currentY < 50.f)
            break;
    }

    int numObstacles = obstacleCountDist(gen);
    float groundY = manager->window.getSize().y - obstacleHeight - 40.f;

    for (int i = 0; i < numObstacles; ++i)
    {
        int retries = 10;
        bool placed = false;

        while (retries-- > 0 && !placed)
        {
            float x = obstacleXDist(gen);
            sf::Vector2f position(x, groundY);
            sf::FloatRect candidateRect(
                sf::Vector2f(x, groundY),
                sf::Vector2f(obstacleWidth, obstacleHeight));

            bool overlaps = false;
            for (const auto &rect : placedRects)
            {
                if (candidateRect.findIntersection(rect))
                {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps)
            {
                entities.push_back(std::make_unique<Destructible>(
                    position, sf::Vector2f(obstacleWidth, obstacleHeight), 100,
                    "assets/vertialObstacle.png"));
                placedRects.push_back(candidateRect);
                placed = true;
            }
        }
    }
}

void ArenaState::update(GameManager &manager, float dt)
{

    if (player->health <= 0)
    {
        manager.window.clear();

        sf::Text text(font, "Unfortunately, you lost!");
        text.setOrigin(text.getLocalBounds().size / 2.f);
        text.setPosition(static_cast<sf::Vector2f>(manager.window.getSize()) *
                         0.5f);
        manager.window.draw(text);
        exitGameBtn.render(manager.window);
        hasEnded = true;
    }

    if (enemy->health <= 0)
    {
        manager.window.clear();

        sf::Text text(font, "Enemy slayed!");
        text.setOrigin(text.getLocalBounds().size / 2.f);
        text.setPosition(static_cast<sf::Vector2f>(manager.window.getSize()) *
                         0.5f);
        manager.window.draw(text);
        continueBtn.render(manager.window);
        if (!hasEnded)
        {
            manager.playerMoney += 10;
            manager.slayedEnemies += 1;
        }
        hasEnded = true;
        return;
    }
    processInput();
    player->setInput(input);

    // shooting logic //
    if (isShooting && player->canShoot)
    {
        auto arrow = std::make_unique<Arrow>(
            player->getRigidbody()->position -
                shooting_dir.normalized() * player->bow.distance,
            entities, player, RigidBody::LAYER_PLAYER_ARROW,
            RigidBody::LAYER_ENEMY_ARROW | RigidBody::LAYER_ENEMY |
                RigidBody::LAYER_ENVIRONMENT | RigidBody::LAYER_PLAYER_ARROW,
            "assets/arrow.png");
        sf::Vector2f direction = static_cast<sf::Vector2f>(shooting_start_pos) -
                                 arrow->getRigidbody()->position;
        float targetAngle = std::atan2(direction.y, direction.x);
        arrow->getRigidbody()->rotation = targetAngle;
        arrow->getRigidbody()->addForce(shooting_dir * -5000.f);
        entities.push_back(std::move(arrow));
        player->canShoot = false;
        isShooting = false;
    }

    std::erase_if(entities, [](const std::unique_ptr<Entity> &e)
                  { return e->toBeDeleted; });

    for (auto &e : entities)
    {
        e->getRigidbody()->isColliding = false;
    }

    // collision detection //
    CollisionResolver::detectCollisionsMultithreaded(entities, contacts);

    // collision resolution //
    for (size_t i = 0; i < contacts.size(); ++i)
    {
        CollisionResolver::resolveContact(contacts[i]);
    }

    // entity updates //
    for (size_t i = 0; i < entities.size(); i++)
    {
        entities[i]->update(dt);
    }

    contacts.clear();
}
