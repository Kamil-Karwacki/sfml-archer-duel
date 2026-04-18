#pragma once
#include "Player.h"

class GameManager;

class AIEnemy : public Player
{
  private:
    GameManager *manager;

  public:
    sf::Vector2f calculateArrowVelocity(const sf::Vector2f &from,
                                        const sf::Vector2f &to, float gravity,
                                        float time);

    void tryShootAtPlayer(Player &player);
    Player *player;
    AIEnemy(float health, sf::Vector2f position,
            std::vector<std::unique_ptr<Entity>> *entities, Player *player,
            GameManager *manager,
            std::string textureFile = "assets/dev_texture.png");
    void update(float dt) override;
};
