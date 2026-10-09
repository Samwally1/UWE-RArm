#pragma once

#include <Arduino.h>

#include "RobotCharacteristics.h"

struct StickInput {
    float leftX;
    float leftY;
    float rightX;
    float rightY;
};

struct RobotPosition {
    float x;
    float y;
    float z;
    float angle;
};

inline float ApplyStickDeadzone(float value) {
    return abs(value) < 0.2f ? 0.0f : value;
}

inline RobotPosition SticksToXYZ(
    const StickInput& input,
    float elapsedMilliseconds,
    RobotPosition position
) {
    const float elapsedSeconds = elapsedMilliseconds / 1000.0f;

    position.x += ApplyStickDeadzone(input.leftX) * ChordSense * elapsedSeconds;
    position.y += ApplyStickDeadzone(input.leftY) * ChordSense * elapsedSeconds;
    position.z += ApplyStickDeadzone(input.rightY) * ChordSense * elapsedSeconds;
    position.angle += ApplyStickDeadzone(input.rightX) * AngleSense * elapsedSeconds;

    return position;
}
