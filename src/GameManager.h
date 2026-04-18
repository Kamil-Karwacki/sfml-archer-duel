#pragma once
#include "GameState.h"

class GameManager
{
  private:
    double deltaTime;

    std::unique_ptr<GameState> currentState;
    std::unique_ptr<GameState> nextState;

    void loop();
    void drawLoop();
    void update();

  public:
    std::string playerName = "gracz";
    uint32_t playerMoney = 0;
    uint32_t upgradeCost = 10;
    uint32_t slayedEnemies = 0;
    float enemyInnacuracy = 0.18f;

    sf::RenderWindow window;
    Player *player;
    AIEnemy *enemy;
    void run();
    void setState(std::unique_ptr<GameState> newState);
};
