#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

#include "Secrets.h"

bool ConnectToWiFi() {

    // Print a message to the Serial Monitor.
    Serial.println("Connecting to WiFi...");

    // Set WiFi to station mode so the board connects to a router.
    WiFi.mode(WIFI_STA);

    // Start connecting using the WiFi name and password.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Record the start time in milliseconds to check for a timeout.
    unsigned long StartTime = millis();

    // Wait until the WiFi connection is established.
    while (WiFi.status() != WL_CONNECTED) {

        // Stop trying after 15 seconds and report failure.
        if (millis() - StartTime >= 15000) {
            Serial.println("\nWiFi connection failed!");
            return false;
        }

        // Wait half a second between connection checks.
        delay(500);

        // Print a dot to show that connection is still in progress.
        Serial.print(".");
    }

    // Report a successful connection.
    Serial.println("\nWiFi connected!");

    // Print the local IP address assigned to the board.
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Return true to indicate a successful connection.
    return true;
}

void TestLatency()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    // Send 3 pings and print average round-trip latency.
    if (Ping.ping(HOST, 3)) {
        Serial.printf("Average latency: %.2f ms\n", Ping.averageTime());
    } else {
        Serial.println("Ping failed");
    }
}

bool CheckControllerMode()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }

    WiFiClient client;

    HTTPClient http;
    const String controllerUrl = "http://" + String(HOST) + ":8000/mode";
    if (!http.begin(client, controllerUrl)) {
        Serial.println("Failed to initialise request");
        return false;
    }

    const char* headerKeys[] = {"mode"};
    http.collectHeaders(headerKeys, 1);
    int statusCode = http.GET();
    bool isTrainingMode = false;

    if (statusCode == HTTP_CODE_OK) {
        const String mode = http.header("mode");
        Serial.printf("Controller mode: %s\n", mode.c_str());
        isTrainingMode = mode == "Training";
    } else if (statusCode > 0) {
        Serial.printf("HTTP error: %d\n", statusCode);
    } else {
        Serial.printf("GET failed: %s\n",
                      HTTPClient::errorToString(statusCode).c_str());
    }

    http.end();
    return isTrainingMode;
}
