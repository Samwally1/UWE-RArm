#pragma once

#include <Arduino.h>
#include <math.h>
#include <limits.h>
#include <iterator>
#include <map>
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
    Serial.printf("[Movement] Packet: %s\n", packet.c_str());

    int firstComma = packet.indexOf(',');
    int secondComma = packet.indexOf(',', firstComma + 1);

    if (firstComma <= 0 || secondComma <= firstComma + 1) {
        Serial.println("[ERROR][Movement] Packet must begin with lines,speed,");
        return false;
    }

    int lines = packet.substring(0, firstComma).toInt();
    int speed = packet.substring(firstComma + 1, secondComma).toInt();

    Serial.printf("[Movement] Lines: %d, speed: %d\n", lines, speed);

    constexpr int MAX_COMMANDS = 10;
    if (lines <= 0 || speed <= 0) {
        Serial.println("[ERROR][Movement] Lines and speed must both be positive");
        return false;
    }
    if (lines > MAX_COMMANDS) {
        Serial.printf("[Movement] Command limit: parsing only the first %d commands\n", MAX_COMMANDS);
    }
    String commands[MAX_COMMANDS];
    const int commandCount = min(lines, MAX_COMMANDS);

    int start = secondComma + 1; // skips lines and speed

    for (int i = 0; i < commandCount; i++) {
        int end = packet.indexOf(',', start);

        if (end == -1) {
            end = packet.length();
        }

        commands[i] = packet.substring(start, end);
        Serial.printf("[Movement] Command %d: %s\n", i + 1, commands[i].c_str());
        start = end + 1;
    }

    // A command is four logical angles in degrees: S1/S2/S3/S4.  Index them
    // by their scheduled offset from the one common movement start time.
    std::map<unsigned long, String> scheduledCommands;
    for (int commandIndex = 0; commandIndex < commandCount; ++commandIndex) {
        scheduledCommands.emplace(static_cast<unsigned long>(commandIndex) * speed,
                                  commands[commandIndex]);
    }

    const unsigned long movementStart = millis();
    while (!scheduledCommands.empty()) {
        const unsigned long elapsed = static_cast<unsigned long>(millis() - movementStart);
        auto command = scheduledCommands.upper_bound(elapsed);
        if (command == scheduledCommands.begin()) {
            delay(1);
            continue;
        }
        --command; // Newest command due at or before the current elapsed time.
        const int commandNumber = static_cast<int>(command->first / speed) + 1;

        float angles[4];
        int valueStart = 0;
        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            const int valueEnd = command->second.indexOf('/', valueStart);
            if ((jointIndex < 3 && valueEnd < 0) || (jointIndex == 3 && valueEnd >= 0)) {
                Serial.printf("[ERROR][Movement] Command %d must contain S1/S2/S3/S4\n",
                    commandNumber);
                return false;
            }

            const String value = command->second.substring(
                valueStart, valueEnd < 0 ? command->second.length() : valueEnd
            );
            char* parseEnd = nullptr;
            angles[jointIndex] = strtof(value.c_str(), &parseEnd);
            if (parseEnd == value.c_str() || *parseEnd != '\0' || !isfinite(angles[jointIndex])
                || angles[jointIndex] < 0.0f || angles[jointIndex] > 180.0f) {
                Serial.printf("[ERROR][Movement] Command %d joint %d has an invalid angle\n",
                    commandNumber, jointIndex + 1);
                return false;
            }
            valueStart = valueEnd + 1;
        }

        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            jointServos[jointIndex].write(ServoCommandAngle(
                jointIndex, static_cast<int>(lroundf(angles[jointIndex]))
            ));
        }
        targetAngles = {
            radians(angles[0]), radians(angles[1]), radians(angles[2]), radians(angles[3])
        };

        // Commands before (and including) the command just applied are stale.
        scheduledCommands.erase(scheduledCommands.begin(), std::next(command));
    }

    return true;
}


