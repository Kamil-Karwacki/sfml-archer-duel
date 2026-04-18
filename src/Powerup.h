#pragma once
#include "Entity.h"

class Player;

class Powerup
{
  public:
    virtual ~Powerup() = default;

    // called when power-up is applied to the player
    virtual void apply(Player &player) = 0;

    // for time-based powerups
    virtual void update(Player &player, float dt) = 0;

    // Called when the effect ends
    virtual void expire(Player &player) = 0;

    virtual bool isExpired() const { return false; }
};

class SpeedPowerup : public Powerup
{
  public:
    float duration = 10.0f;
    float timer = 0.0f;
    float speedMultiplier = 2.0f;
    bool applied = false;
    void apply(Player &player) override;

    void update(Player &player, float dt) override;

    void expire(Player &player) override;

    bool isExpired() const override;
};

class DoubleJumpPowerup : public Powerup
{
  public:
    float duration = 10.0f;
    float timer = 0.0f;
    bool applied = false;
    void apply(Player &player) override;

    void update(Player &player, float dt) override;

    void expire(Player &player) override;

    bool isExpired() const override;
};
class PowerupEntity : public Entity
{
  private:
    std::unique_ptr<Powerup> effect;
    float yLevel;
    float speed = 50.f;
    bool right; // where powerup should move
    float accumulatedTime = 0.f;

  public:
    PowerupEntity(std::unique_ptr<Powerup> _effect, sf::Vector2f position,
                  std::string textureFile = "assets/dev_texture.png");
    void update(float dt) override;

    void onPickup(Player &player);
};
