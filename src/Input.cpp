#include "Input.h"
#include <iostream>

void processInput(sf::Vector2i& input, bool& wasLMBpressed, sf::Vector2i& shooting_start_pos, sf::Vector2i& shooting_end_pos,
                  bool& isAiming, const sf::WindowBase& window, bool& isShooting, bool canShoot)
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

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        input.x = 0;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        input.y = 0;
    }


    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canShoot)
    {
        if (!wasLMBpressed)
        {
            shooting_start_pos = sf::Mouse::getPosition(window);
            std::cout << "Aiming\n";
            isAiming = true;
        }
    }

    if (isAiming && canShoot)
    {
        shooting_end_pos = sf::Mouse::getPosition(window);
        if(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            isAiming = false;
    }

    if (wasLMBpressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canShoot)
    {
        shooting_end_pos = sf::Mouse::getPosition(window);
        std::cout << "Shooting\n";
        isShooting = true;
    }
    wasLMBpressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canShoot;
}
