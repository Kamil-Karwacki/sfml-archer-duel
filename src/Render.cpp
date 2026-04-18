#include "Render.h"

std::unordered_map<std::string, sf::Texture> TextureManager::textures;

sf::Texture &TextureManager::getTexture(const std::string &filename)
{
    auto it = textures.find(filename);
    if (it == textures.end())
    {
        sf::Texture texture;
        if (!texture.loadFromFile(filename))
        {
            throw std::runtime_error("Failed to load texture: " + filename);
        }
        it = textures.emplace(filename, std::move(texture)).first;
    }
    return it->second;
}

DrawableComp::DrawableComp(const std::string &textureFile,
                           sf::Vector2f targetSize)
    : sprite(TextureManager::getTexture(textureFile)), targetSize(targetSize)
{
    sf::Vector2i bounds = sprite.getTextureRect().size;

    sf::Vector2f scale = {targetSize.x / bounds.x, targetSize.y / bounds.y};
    sprite.setScale(scale);

    sprite.setOrigin({bounds.x / 2.f, bounds.y / 2.f});
}

void DrawableComp::draw(sf::RenderTarget &target, sf::RenderStates states)
{
    target.draw(sprite, states);
    return;
#ifdef DEBUG
    sf::CircleShape shape(3.f);
    shape.setFillColor(sf::Color(55, 60, 250));
    shape.setPosition(sprite.getPosition() - targetSize / 2.f);
    // target.draw(shape);

    shape.setFillColor(sf::Color(100, 250, 50));
    shape.setPosition(sprite.getPosition() + targetSize / 2.f);
    // target.draw(shape);

    shape.setFillColor(sf::Color(250, 50, 50));
    shape.setPosition(sprite.getPosition());
    // target.draw(shape);

    auto gb = sprite.getGlobalBounds();
    sf::RectangleShape rect;
    rect.setSize(gb.size);
    rect.setPosition(gb.position);
    rect.setRotation(sprite.getRotation());
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(250, 50, 50));
    target.draw(rect);
#endif // DEBUG
}
