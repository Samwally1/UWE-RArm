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
    PrintControllerState();

    if (ArmMode == "Training") {
        Serial.println("--------------- In Training Mode ---------------");

    }
    delay(1000);
}
