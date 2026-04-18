#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class Button
{
  public:
    Button(const sf::Vector2f &size, const sf::Vector2f &position,
           const std::string &text, const sf::Font &font,
           unsigned int characterSize = 24);

    void setColors(const sf::Color &idle, const sf::Color &hover,
                   const sf::Color &active);
    void setCallback(std::function<void()> onClick);

    void handleEvent(const sf::RenderWindow &window, const sf::Event &event);
    void render(sf::RenderWindow &window);

  private:
    sf::RectangleShape shape;
    sf::Text label;
    std::function<void()> callback;

    enum class State
    {
        Idle,
        Hover,
        Active
    } currentState;

    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color activeColor;

    bool mouseOver(const sf::RenderWindow &window);
};
