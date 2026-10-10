#pragma once

#include "WifiFunctions.h"

inline bool ReadNextCalibrationValue(const char*& cursor, int& value)
{
    while (*cursor != '\0' && *cursor != '-' && (*cursor < '0' || *cursor > '9')) {
        ++cursor;
    }
    if (*cursor == '\0') {
        return false;
    }

    char* valueEnd = nullptr;
    const long parsedValue = strtol(cursor, &valueEnd, 10);
    if (valueEnd == cursor || parsedValue < 0 || parsedValue > 180) {
        return false;
    }

    value = static_cast<int>(parsedValue);
    cursor = valueEnd;
    return true;
}

inline bool LoadCalibration()
{
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    const String calibrationUrl = "https://" + String(HOST) + "/calibration";
    if (!http.begin(client, calibrationUrl)) {
        return false;
    }

    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    const String calibration = http.getString();
    http.end();
    const int limitsKey = calibration.indexOf("\"jointLimits\"");
    const int zerosKey = calibration.indexOf("\"joint0s\"");
    if (limitsKey < 0 || zerosKey < 0) {
        return false;
    }

    const char* cursor = calibration.c_str() + limitsKey;
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (!ReadNextCalibrationValue(cursor, JointLims[jointIndex][0])
            || !ReadNextCalibrationValue(cursor, JointLims[jointIndex][1])) {
            return false;
        }
    }

    cursor = calibration.c_str() + zerosKey;
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (!ReadNextCalibrationValue(cursor, Joint0s[jointIndex])) {
            return false;
        }
    }
    return true;
}

inline bool SaveCalibration()
{
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    String payload = "{\"jointLimits\":[";
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (jointIndex > 0) {
            payload += ',';
        }
        payload += "[" + String(JointLims[jointIndex][0]) + "," + String(JointLims[jointIndex][1]) + "]";
    }
    payload += "],\"joint0s\":[";
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (jointIndex > 0) {
            payload += ',';
        }
        payload += String(Joint0s[jointIndex]);
    }
    payload += "]}";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    const String calibrationUrl = "https://" + String(HOST) + "/calibration";
    if (!http.begin(client, calibrationUrl)) {
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    const int statusCode = http.POST(payload);
    http.end();
    return statusCode == HTTP_CODE_NO_CONTENT;
}

inline String CalibrationStatus()
{
    String status = "Loaded calibration | Joint 0s: ";
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        if (jointIndex > 0) {
            status += ", ";
        }
        status += String(Joint0s[jointIndex]);
    }
    return status;
}
