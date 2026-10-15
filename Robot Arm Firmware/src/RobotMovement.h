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
        const int logicalAngle = constrain(
            static_cast<int>(lroundf(angleDegrees)), minimum, maximum
        );
        jointServos[jointIndex].write(
            ServoCommandAngle(jointIndex, logicalAngle)
        );
    }
}


// speedMmPerSecond controls the Cartesian travel speed, in millimetres/second.
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
    const float moveDurationMs = distanceMm * 1000.0f / speedMmPerSecond;
    Serial.printf(
        "Linear move: %.1f mm at %.1f mm/s (%.1f s)\\n",
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

        const RobotPosition targetPosition = {
            currentPosition.x + deltaX * progress,
            currentPosition.y + deltaY * progress,
            currentPosition.z + deltaZ * progress,
            currentPosition.angle + deltaA * progress,
        };

        RobotAngles nextAngles;
        if (InverseKinTransform(targetPosition, targetAngles, nextAngles)) {

            CommandJointAngles(nextAngles);

            targetAngles = nextAngles;
        } else {
            Serial.printf(
                "Linear movement path is unreachable at %.0f%%\\n",
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
