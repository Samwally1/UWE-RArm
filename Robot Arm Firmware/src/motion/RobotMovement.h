#pragma once

#include <Arduino.h>
#include <math.h>
#include <limits.h>
#include <ESP32Servo.h>
#include "../config/RobotCharacteristics.h"
#include "../core/RobotTypes.h"


Servo jointServos[4];

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};

RobotAngles targetAngles = {HALF_PI, HALF_PI, 0.0f, 0.0f};

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

inline bool ExecuteMovementPacket(const String& packet)
{
    int firstComma = packet.indexOf(',');
    int secondComma = packet.indexOf(',', firstComma + 1);

    int lines = packet.substring(0, firstComma).toInt();
    int speed = packet.substring(firstComma + 1, secondComma).toInt();

    constexpr int MAX_COMMANDS = 10;
    String commands[MAX_COMMANDS];

    int start = secondComma + 1; // skips lines and speed

    for (int i = 0; i < lines && i < MAX_COMMANDS; i++) {
        int end = packet.indexOf(',', start);

        if (end == -1) {
            end = packet.length();
        }

        commands[i] = packet.substring(start, end);
        start = end + 1;
    }

    return true;
    
}


