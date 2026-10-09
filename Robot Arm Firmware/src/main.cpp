#include <Arduino.h>
#include "WifiFunctions.h"
#include "SticksToXYZ.h"

String ArmMode = "";

StickInput controllerInput = {0, 0, 0, 0};

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};

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
    static unsigned long lastPositionUpdateTime = 0;
    static unsigned long lastPositionPrintTime = 0;

    if (ArmMode == "Training") {

        if (PrintControllerState(controllerInput)) {

            const unsigned long currentTime = millis();

            targetPosition = SticksToXYZ(
                controllerInput,
                currentTime - lastPositionUpdateTime,
                targetPosition
            );

            Serial.printf(
                "Target XYZ: %.2f, %.2f, %.2f | Angle: %.2f\n",
                targetPosition.x,
                targetPosition.y,
                targetPosition.z,
                targetPosition.angle
            );

           
        } else {
            // Reset timing when controller input is unavailable.
            lastPositionUpdateTime = 0;
        }
    } else {
        lastPositionUpdateTime = 0;
    }

    delay(1);
}