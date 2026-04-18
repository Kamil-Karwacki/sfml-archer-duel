#pragma once
#include "GameState.h"

class CharacterCreationState : public GameState
{
  public:
    GameManager *manager = nullptr;
    sf::Font font;
    sf::Text titleText;
    std::string playerInput = "";
    sf::Text characterName;
    CharacterCreationState(GameManager *manager);
    void handleEvent(GameManager &manager, sf::Event &event) override;
    void update(GameManager &manager, float dt) override;
    void draw(GameManager &manager, sf::RenderWindow &window) override;
};
