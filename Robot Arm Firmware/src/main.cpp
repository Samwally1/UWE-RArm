#include <Arduino.h>
#include "WifiFunctions.h"

String ArmMode = "";

void setup()
{
    Serial.begin(9600);

    if (ConnectToWiFi()) {
        TestLatency();
        ArmMode = CheckControllerMode();
    }
}

void loop()
{
    static unsigned long lastModeMessageTime = 0;

    PrintControllerState();

    if (ArmMode == "Training" && millis() - lastModeMessageTime >= 1000) {
        lastModeMessageTime = millis();
    }

    delay(1);
}
