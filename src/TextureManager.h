#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <string>

class TextureManager
{
  private:
    std::map<std::string, std::shared_ptr<sf::Texture>> textures;

  public:
    bool LoadTexture(const std::string &name, const std::string &filename)
    {
        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(filename))
        {
            return false;
        }
        textures[name] = texture;
        return true;
    }

    std::shared_ptr<sf::Texture> GetTexture(const std::string &name)
    {
        return textures.at(name);
    }
};
