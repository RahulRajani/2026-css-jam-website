#include "chronos.h"
#include <raymath.h>

void updateChronos(Chronos& chronos, float deltaTime, bool dangerous) {
    Vector2 target = chronos.movingToB
        ? chronos.waypointB
        : chronos.waypointA;

    Vector2 direction = Vector2Subtract(target, chronos.position);
    float distance = Vector2Length(direction);
    float speed = dangerous ? chronos.dangerSpeed : chronos.normalSpeed;
    float step = speed * deltaTime;

    if (distance <= step || distance <= 0.001f) {
        chronos.position = target;
        chronos.movingToB = !chronos.movingToB;
        return;
    }

    chronos.position = Vector2Add(
        chronos.position,
        Vector2Scale(Vector2Normalize(direction), step)
    );
}
