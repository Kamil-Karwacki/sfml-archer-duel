#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class TextureManager
{
  private:
    static std::unordered_map<std::string, sf::Texture> textures;

  public:
    static sf::Texture &getTexture(const std::string &filename);
};

class DrawableComp
{
  public:
    sf::Sprite sprite;
    sf::Vector2f targetSize;

    DrawableComp(const std::string &textureFile,
                 sf::Vector2f targetSize = {64.f, 64.f});

    void draw(sf::RenderTarget &target,
              sf::RenderStates states = sf::RenderStates::Default);
};
