#pragma once
#include "Button.h"
#include "GameState.h"

class BuyingMenuState : public GameState
{
  public:
    GameManager *manager = nullptr;
    sf::Font font;
    Button upgradeBowButton;
    Button goFightButton;
    Button saveAndExitButton;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    BuyingMenuState(GameManager *manager);
    void handleEvent(GameManager &manager, sf::Event &event) override;
    void update(GameManager &, float) override {}
    void draw(GameManager &, sf::RenderWindow &window) override;
};
