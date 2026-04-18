#include "CharacterCreationState.h"
#include "ArenaState.h"
#include "FileIO.h"
#include "GameManager.h"
#include "GameState.h"
#include <regex>

CharacterCreationState::CharacterCreationState(GameManager *manager)
    : manager(manager), font("assets/CONSOLA.TTF"),
      titleText(font, "Type in your characters name\nPress enter to confirm"),
      characterName(font, "")
{
    titleText.setOrigin(titleText.getLocalBounds().size / 2.f);
    titleText.setPosition(static_cast<sf::Vector2f>(manager->window.getSize()) /
                          2.f);

    characterName.setOrigin(titleText.getLocalBounds().size / 2.f);
    characterName.setPosition(
        (static_cast<sf::Vector2f>(manager->window.getSize()) / 2.f) +
        sf::Vector2f(0, manager->window.getSize().y / 3.f));
}

void CharacterCreationState::handleEvent(GameManager &manager, sf::Event &event)
{
    if (auto textEvent = event.getIf<sf::Event::TextEntered>())
    {
        if (textEvent->unicode == 13)
        {
            manager.playerName = playerInput;
            manager.setState(std::make_unique<ArenaState>(&manager));
            return;
        }
        playerInput += static_cast<char>(textEvent->unicode);
        std::regex disallowed("[^a-zA-Z0-9 ]");
        playerInput = std::regex_replace(playerInput, disallowed, "");
        characterName.setString(playerInput);
    }
}

void CharacterCreationState::update(GameManager &manager, float dt) {}

void CharacterCreationState::draw(GameManager &manager,
                                  sf::RenderWindow &window)
{
    window.draw(titleText);
    window.draw(characterName);
}
