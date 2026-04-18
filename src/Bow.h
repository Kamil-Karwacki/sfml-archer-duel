#pragma once
#include "Render.h"

struct Bow
{
    std::string name = "Placeholder";
    float arrowDamage = 0;
    float reloadTime = 0;
    std::unique_ptr<DrawableComp> drawable = nullptr;
    float degreeOffset = 0;
    float distance = 0; // distance from player

    Bow() = default;
    Bow(std::string name, float arrowDamage, float reloadTime,
        float degreeOffset, float distanceFromPlayer,
        std::unique_ptr<DrawableComp> drawable)
        : name(name), arrowDamage(arrowDamage), reloadTime(reloadTime),
          degreeOffset(degreeOffset), distance(distanceFromPlayer),
          drawable(std::move(drawable))
    {
    }
};
