#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

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

    String pingHost = String(HOST);
    pingHost.replace("https://", "");
    pingHost.replace("http://", "");
    const int pathStart = pingHost.indexOf('/');
    if (pathStart >= 0) {
        pingHost.remove(pathStart);
    }

    // Ping only the host name; Ping cannot resolve a URL with a protocol or path.
    if (Ping.ping(pingHost.c_str(), 3)) {
        Serial.printf("Average latency: %.2f ms\n", Ping.averageTime());
    } else {
        Serial.println("Ping failed");
    }
}

String CheckControllerMode()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return "";
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    // HOST should contain only the hostname, without https://.
    String controllerUrl = "https://" + String(HOST) + "/mode";

    if (!http.begin(client, controllerUrl)) {
        Serial.println("Failed to initialise request");
        return "";
    }

    const char* headerKeys[] = {"mode"};
    http.collectHeaders(headerKeys, 1);

    int statusCode = http.GET();
    String mode = "";

    if (statusCode == HTTP_CODE_OK) {
        mode = http.header("mode");

        Serial.printf("Controller mode: %s\n", mode.c_str());
    }
    else if (statusCode > 0) {
        Serial.printf("HTTP error: %d\n", statusCode);
    }
    else {
        Serial.printf(
            "GET failed: %s\n",
            HTTPClient::errorToString(statusCode).c_str()
        );
    }

    http.end();

    return mode;
}

void PrintControllerState()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    const String controllerUrl = "https://" + String(HOST) + "/controller";
    if (!http.begin(client, controllerUrl)) {
        Serial.println("Failed to initialise controller request");
        return;
    }

    const int statusCode = http.GET();
    if (statusCode == HTTP_CODE_OK) {
        Serial.print("Controller state: ");
        Serial.println(http.getString());
    } else if (statusCode > 0) {
        Serial.printf("Controller HTTP error: %d\n", statusCode);
    } else {
        Serial.printf(
            "Controller GET failed: %s\n",
            HTTPClient::errorToString(statusCode).c_str()
        );
    }

    http.end();
}
