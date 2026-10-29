#include "core/StartupProcess.h"
#include "network/MovementClient.h"

void setup()
{
    StartupProcess();
}

void loop()
{
    String movementPacket;
    if (GetMovementPacket(targetAngles, movementPacket)) {
        if (!ExecuteMovementPacket(movementPacket, targetAngles)) {
            Serial.println("[ERROR][Movement] Playback aborted; retaining last commanded angles");
        }
    }
    // Avoid immediately re-requesting a packet after a 204 or failed request.
    delay(100);
}
