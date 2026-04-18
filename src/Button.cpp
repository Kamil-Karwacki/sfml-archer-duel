#include "Button.h"
#include "SFML/Window/Mouse.hpp"

Button::Button(const sf::Vector2f &size, const sf::Vector2f &position,
               const std::string &text, const sf::Font &_font,
               unsigned int characterSize)
    : label(sf::Text(_font, text, characterSize))
{
    shape = sf::RectangleShape(size);
    shape.setPosition(position);

    label.setFillColor(sf::Color::White);

    label.setPosition(position);
    currentState = State::Idle;
    idleColor = sf::Color(50, 50, 50);
    hoverColor = sf::Color(150, 150, 150);
    activeColor = sf::Color(200, 200, 200);

    shape.setFillColor(idleColor);
}

void Button::setColors(const sf::Color &idle, const sf::Color &hover,
                       const sf::Color &active)
{
    idleColor = idle;
    hoverColor = hover;
    activeColor = active;
}

void Button::setCallback(std::function<void()> onClick)
{
    callback = std::move(onClick);
}

bool Button::mouseOver(const sf::RenderWindow &window)
{
    auto mousePos = sf::Mouse::getPosition(window);
    return shape.getGlobalBounds().contains(
        static_cast<sf::Vector2f>(mousePos));
}

void Button::handleEvent(const sf::RenderWindow &window, const sf::Event &event)
{
    if (mouseOver(window))
    {
        currentState = State::Hover;

        if (event.is<sf::Event::MouseButtonPressed>() &&
            sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            currentState = State::Active;
            if (callback)
                callback();
        }
    }
    else
    {
        currentState = State::Idle;
    }

    switch (currentState)
    {
    case State::Idle:
        shape.setFillColor(idleColor);
        break;
    case State::Hover:
        shape.setFillColor(hoverColor);
        break;
    case State::Active:
        shape.setFillColor(activeColor);
        break;
    }
}

void Button::render(sf::RenderWindow &window)
{
    window.draw(shape);
    window.draw(label);
}
