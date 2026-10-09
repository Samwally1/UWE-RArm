#include <Arduino.h>
#include "WifiFunctions.h"
#include "SticksToXYZ.h"

String ArmMode = "";
String ArmStatus = "Starting";

static unsigned long lastPositionUpdateTime = 0;
static unsigned long lastPositionPrintTime = 0;

StickInput controllerInput = {0, 0, 0, 0, 0};

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};



void setup()
{   
    Serial.begin(9600);

    ConnectToWiFi();
    TestLatency();
    ArmMode = CheckControllerMode();

    if (ArmMode == "Training") {

        while (controllerInput.aButton == 0) {

        if (GetControllerState(controllerInput)) {

            const unsigned long currentTime = millis();

            if (lastPositionUpdateTime != 0) {
                targetPosition = SticksToXYZ(
                    controllerInput,
                    currentTime - lastPositionUpdateTime,
                    targetPosition
                );
            }

            lastPositionUpdateTime = currentTime;

            if (currentTime - lastPositionPrintTime >= 100) {
                Serial.printf(
                    "Target XYZ: %.2f, %.2f, %.2f | Angle: %.2f\n",
                    targetPosition.x,
                    targetPosition.y,
                    targetPosition.z,
                    targetPosition.angle
                );
                lastPositionPrintTime = currentTime;
            }

        
        } else {
            // Reset timing when controller input is unavailable.
            lastPositionUpdateTime = 0;
            }
        }

    }

}


void loop()
{
    SendArmStatus(ArmStatus);
    Serial.print("x");
    delay(10000);
}




