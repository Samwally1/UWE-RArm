#include "core/StartupProcess.h"
#include "network/MovementClient.h"

void setup()
{
    StartupProcess();
    Serial.println("[Movement] Continuously checking backend for target movements");
}

void loop()
{
    static bool completionPending = false;
    static unsigned long lastRequestTime = 0;
    static unsigned long lastReconnectTime = 0;
    static bool requested = false;
    static bool reconnectRequested = false;
    const unsigned long now = millis();

    if (WiFi.status() != WL_CONNECTED) {
        if (!reconnectRequested || now - lastReconnectTime >= 5000) {
            lastReconnectTime = now;
            reconnectRequested = true;
            Serial.println("[Movement] Reconnecting to WiFi");
            WiFi.reconnect();
        }
        delay(1);
        return;
    }
    reconnectRequested = false;

    // Complete the previous packet before requesting another. Retry failed
    // acknowledgements once per second; idle target checks run every 100 ms.
    const unsigned long interval = completionPending ? 1000 : 100;
    if (requested && now - lastRequestTime < interval) {
        delay(1);
        return;
    }
    requested = true;
    lastRequestTime = now;

    if (completionPending) {
        if (!SendMovementComplete()) {
            return;
        }
        completionPending = false;
    }

    String movementPacket;
    if (GetMovementPacket(targetAngles, movementPacket)) {
        Serial.printf("[Movement] Received packet: %u bytes\n", movementPacket.length());
        if (ExecuteMovementPacket(movementPacket, targetAngles)) {
            completionPending = true;
            // Attempt completion immediately on the next loop iteration.
            requested = false;
        } else {
            Serial.println("[ERROR][Movement] Playback aborted; retaining last commanded angles");
        }
    }
}
