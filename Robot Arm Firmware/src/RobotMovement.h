#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "WifiFunctions.h"
#include "SticksToXYZ.h"
#include "InverseKinTransform.h"
#include "CalibrationFuncs.h"


Servo jointServos[4];

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};

RobotAngles targetAngles = {HALF_PI, HALF_PI, 0.0f, 0.0f};

float JointAngleAt(const RobotAngles& angles, int jointIndex)
{
    switch (jointIndex) {
        case 0: return angles.s1;
        case 1: return angles.s2;
        case 2: return angles.s3;
        default: return angles.s4;
    }
}

RobotAngles HomeJointAngles()
{
    return {
        static_cast<float>(radians(JointHomes[0])),
        static_cast<float>(radians(JointHomes[1])),
        static_cast<float>(radians(JointHomes[2])),
        static_cast<float>(radians(JointHomes[3])),
    };
}

int ServoCommandAngle(int jointIndex, int logicalAngle)
{
    const int clampedAngle = constrain(logicalAngle, 0, 180);
    // A reversed calibration range marks an inverted servo: logical 0 degrees
    // maps to the physical 180-degree command, and vice versa.
    const bool inverted = JointLims[jointIndex][0] > JointLims[jointIndex][1];
    return inverted ? 180 - clampedAngle : clampedAngle;
}

void CommandJointAngles(const RobotAngles& angles)
{
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (!jointServos[jointIndex].attached()) {
            jointServos[jointIndex].setPeriodHertz(50);
            jointServos[jointIndex].attach(JointPins[jointIndex], 500, 2400);
        }
        const float angleDegrees = degrees(JointAngleAt(angles, jointIndex));
        const int minimum = min(JointLims[jointIndex][0],
                                JointLims[jointIndex][1]);
        const int maximum = max(JointLims[jointIndex][0],
                                JointLims[jointIndex][1]);
        const float logicalAngle = constrain(
            angleDegrees, static_cast<float>(max(0, minimum)),
            static_cast<float>(min(180, maximum))
        );
        const bool inverted = JointLims[jointIndex][0] > JointLims[jointIndex][1];
        const float servoAngle = inverted ? 180.0f - logicalAngle : logicalAngle;
        // Preserve fractional degrees instead of stepping in whole degrees.
        const int pulseUs = static_cast<int>(lroundf(
            500.0f + servoAngle * (2400.0f - 500.0f) / 180.0f
        ));
        jointServos[jointIndex].writeMicroseconds(pulseUs);
    }
}


// speedMmPerSecond caps the peak Cartesian speed, in millimetres/second.
bool CommandJointAnglesLinear(const RobotPosition& positions,
                              const RobotPosition& prevpositions,
                              float speedMmPerSecond)
{

    if (speedMmPerSecond <= 0.0f) {
        Serial.println("Linear movement failed: speed must be above zero");
        return false;
    }

    const RobotAngles currentAngles = targetAngles;
    const RobotPosition currentPosition = prevpositions;

    const float deltaX = positions.x - prevpositions.x;
    const float deltaY = positions.y - prevpositions.y;
    const float deltaZ = positions.z - prevpositions.z;
    const float deltaA = positions.angle - prevpositions.angle;


    const float distanceMm = sqrtf(
        deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ
    );
    constexpr unsigned long controlIntervalMs = 20;
    // Quintic easing has a peak slope of 1.875. Extend the duration so the
    // requested speed remains the peak speed, with zero speed/acceleration
    // at both ends. Also give orientation-only moves a gradual transition.
    const float moveDurationMs = max(
        500.0f, max(1.875f * distanceMm * 1000.0f / speedMmPerSecond,
                    1.875f * fabsf(deltaA) * 1000.0f / 30.0f)
    );
    Serial.printf(
        "Linear move: %.1f mm at %.1f mm/s (%.1f s)\n",
        distanceMm, speedMmPerSecond, moveDurationMs / 1000.0f
    );

    
    
    RobotAngles finalAngles;
    if (!InverseKinTransform(positions, currentAngles, finalAngles)) {
        Serial.println("Linear movement target is unreachable");
        return false;
    }
    const unsigned long startTime = millis();

    
    while (true) {
        const unsigned long elapsedMs = millis() - startTime;
        const float progress = moveDurationMs > 0.0f
            ? min(1.0f, elapsedMs / moveDurationMs)
            : 1.0f;

        const float easedProgress = progress * progress * progress
            * (10.0f + progress * (-15.0f + 6.0f * progress));

        const RobotPosition targetPosition = {
            currentPosition.x + deltaX * easedProgress,
            currentPosition.y + deltaY * easedProgress,
            currentPosition.z + deltaZ * easedProgress,
            currentPosition.angle + deltaA * easedProgress,
        };

        RobotAngles nextAngles;
        if (InverseKinTransform(targetPosition, targetAngles, nextAngles)) {

            CommandJointAngles(nextAngles);

            targetAngles = nextAngles;
        } else {
            Serial.printf(
                "Linear movement path is unreachable at %.0f%%\n",
                progress * 100.0f
            );
            return false;
        }

        if (progress >= 1.0f) {
            break;
        }
        delay(controlIntervalMs);
    }

    return true;
}
