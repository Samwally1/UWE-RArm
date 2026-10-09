#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h"



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