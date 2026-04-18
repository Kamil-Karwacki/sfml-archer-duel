#pragma once
#include <SFML/Graphics.hpp>

void processInput(sf::Vector2i& input, bool& wasLMBPressed, sf::Vector2i& shooting_start_pos, sf::Vector2i& shooting_end_pos,
	bool& isAiming, const sf::WindowBase& window, bool& isShooting, bool canShoot);