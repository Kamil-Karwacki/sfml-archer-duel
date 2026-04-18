#pragma once
#include "Button.h"
#include "GameState.h"

class MainMenuState : public GameState
{
  public:
    MainMenuState(GameManager *manager);
    GameManager *manager = nullptr;
    sf::Font font;
    Button startNewGameBtn;
    Button loadGameBtn;
    Button exitBtn;
    Button returnBtn;
    std::vector<Button> saveGamesBtns;
    bool loadingSaves = false;

    void handleEvent(GameManager &manager, sf::Event &event) override;
    void update(GameManager &, float) override {}
    void draw(GameManager &manager, sf::RenderWindow &window) override;
};
