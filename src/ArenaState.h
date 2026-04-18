#pragma once
#include "Button.h"
#include "GameState.h"

class ArenaState : public GameState
{
  public:
    GameManager *manager = nullptr;

    std::vector<std::unique_ptr<Entity>> entities;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    Player *player;
    AIEnemy *enemy;

    sf::Vector2i input = {0, 0};
    std::vector<Contact> contacts;
    bool hasEnded = false;

    bool wasLMBpressed = false;
    sf::Vector2i shooting_start_pos;
    sf::Vector2i shooting_end_pos;
    sf::Vector2f shooting_dir;
    bool isAiming = false;
    bool isShooting = false;

    sf::Font font;
    Button exitGameBtn;
    Button continueBtn;
    ArenaState(GameManager *_manager);
    void handleEvent(GameManager &manager, sf::Event &event) override;
    void update(GameManager &manager, float dt) override;
    void draw(GameManager &manager, sf::RenderWindow &window) override;
    void processInput();
    void generateFloatingPlatforms();
};
