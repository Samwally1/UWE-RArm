#pragma once

#include <Arduino.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
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

// Packet: step count, interval in milliseconds, then steps + 1 timestamped rows.
inline bool ReadMovementUnsigned(const String& text, unsigned long& value)
{
    if (text.length() == 0) return false;
    for (unsigned int i = 0; i < text.length(); ++i) {
        if (text[i] < '0' || text[i] > '9') return false;
    }
    errno = 0;
    char* end = nullptr;
    value = strtoul(text.c_str(), &end, 10);
    return errno != ERANGE && *end == '\0';
}

inline bool ReadMovementLine(const String& packet, int& cursor, String& line)
{
    if (cursor >= static_cast<int>(packet.length())) return false;
    int end = packet.indexOf('\n', cursor);
    if (end < 0) end = packet.length();
    line = packet.substring(cursor, end);
    line.trim();
    cursor = end + 1;
    return true;
}

inline bool ExecuteMovementPacket(const String& packet)
{
    int cursor = 0;
    String line;
    unsigned long steps, interval;
    if (!ReadMovementLine(packet, cursor, line) || !ReadMovementUnsigned(line, steps)
        || steps == 0 || steps > packet.length()
        || !ReadMovementLine(packet, cursor, line) || !ReadMovementUnsigned(line, interval)) {
        Serial.println("[ERROR][Movement] Expected step count and interval on separate lines");
        return false;
    }

    // Validate the entire packet before issuing any servo commands.
    const int rowsStart = cursor;
    for (int pass = 0; pass < 2; ++pass) {
        cursor = rowsStart;
        unsigned long previousTime = 0;
        const unsigned long movementStart = millis();
        for (unsigned long row = 0; row <= steps; ++row) {
            if (!ReadMovementLine(packet, cursor, line)) {
                Serial.println("[ERROR][Movement] Missing command row");
                return false;
            }
            int separator = line.indexOf('/');
            unsigned long timestamp;
            if (separator <= 0
                || !ReadMovementUnsigned(line.substring(0, separator), timestamp)
                || timestamp > LONG_MAX || (row == 0 && timestamp != 0)
                || timestamp < previousTime) {
                Serial.println("[ERROR][Movement] Invalid command timestamp");
                return false;
            }
            previousTime = timestamp;
            float angles[4];
            int start = separator + 1;
            for (int joint = 0; joint < 4; ++joint) {
                int end = line.indexOf('/', start);
                if ((joint < 3 && end < 0) || (joint == 3 && end >= 0)) {
                    Serial.println("[ERROR][Movement] Expected timestamp/S1/S2/S3/S4");
                    return false;
                }
                const String value = line.substring(start, end < 0 ? line.length() : end);
                char* parseEnd = nullptr;
                angles[joint] = strtof(value.c_str(), &parseEnd);
                if (parseEnd == value.c_str() || *parseEnd != '\0'
                    || !isfinite(angles[joint]) || angles[joint] < 0 || angles[joint] > 180) {
                    Serial.println("[ERROR][Movement] Invalid joint angle");
                    return false;
                }
                start = end + 1;
            }
            if (pass == 1) {
                while (static_cast<unsigned long>(millis() - movementStart) < timestamp) {
                    delay(1);
                }
                for (int joint = 0; joint < 4; ++joint) {
                    jointServos[joint].write(ServoCommandAngle(
                        joint, static_cast<int>(lroundf(angles[joint]))));
                }
                targetAngles = {static_cast<float>(radians(angles[0])), static_cast<float>(radians(angles[1])),
                                static_cast<float>(radians(angles[2])), static_cast<float>(radians(angles[3]))};
            }
        }
        if (packet.substring(cursor).length() > 0) {
            String remainder = packet.substring(cursor);
            remainder.trim();
            if (remainder.length() > 0) {
                Serial.println("[ERROR][Movement] Unexpected extra command rows");
                return false;
            }
        }
    }
    return true;
}
