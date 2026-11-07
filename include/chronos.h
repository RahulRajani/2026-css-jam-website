#pragma once

#include <raylib.h>

struct Chronos {
    Vector2 position{40.0f, 40.0f};
    Vector2 waypointA{40.0f, 40.0f};
    Vector2 waypointB{200.0f, 40.0f};
    float normalSpeed = 25.0f;
    float dangerSpeed = 55.0f;
    bool movingToB = true;
};

void updateChronos(Chronos& chronos, float deltaTime, bool dangerous);
