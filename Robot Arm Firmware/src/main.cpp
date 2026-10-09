#include <Arduino.h>
#include <ESP32Servo.h>
#include "WifiFunctions.h"
#include "SticksToXYZ.h"

String ArmMode = "";
String ArmStatus = "Starting";

static unsigned long lastPositionUpdateTime = 0;
static unsigned long lastPositionPrintTime = 0;

StickInput controllerInput = {0, 0, 0, 0, 0};
Servo jointServos[4];

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};



void setup()
{   
    Serial.begin(9600);

    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        jointServos[jointIndex].setPeriodHertz(50);
        jointServos[jointIndex].attach(JointPins[jointIndex], 500, 2400);
        jointServos[jointIndex].write(90);
    }

    ConnectToWiFi();
    TestLatency();
    ArmMode = CheckControllerMode();

    if (ArmMode == "Training") {
        SendArmStatus("Training Angles");

        const char* operationNames[4] = {
            "Min angle",
            "Max angle",
            "0 angle",
            "90 angle",
        };

        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            int angleOutputs[4] = {0, 0, 0, 0};

            for (int operationIndex = 0; operationIndex < 4; ++operationIndex) {
                SendArmStatus(
                    "Joint " + String(jointIndex + 1) + ": " + operationNames[operationIndex]
                );

                while (controllerInput.aButton == 0) {
                    if (!GetControllerState(controllerInput)) {
                        continue;
                    }

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
                        const int servoAngle = constrain(
                            static_cast<int>(targetPosition.angle),
                            0,
                            180
                        );
                        jointServos[jointIndex].write(servoAngle);
                        Serial.printf(
                            "Angle: %d\n",
                            servoAngle
                        );
                        lastPositionPrintTime = currentTime;
                    }
                }

                angleOutputs[operationIndex] = static_cast<int>(targetPosition.angle);

                while (controllerInput.aButton == 1) {
                    GetControllerState(controllerInput);
                }
            }

            JointLims[jointIndex][0] = angleOutputs[0];
            JointLims[jointIndex][1] = angleOutputs[1];
            Joint0s[jointIndex] = angleOutputs[2];
            Joint90s[jointIndex] = angleOutputs[3];
        }
    }

    Serial.println("Joint calibration:");
    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        Serial.printf(
            "Joint %d | Min: %d | Max: %d | 0 deg: %d | 90 deg: %d\n",
            jointIndex + 1,
            JointLims[jointIndex][0],
            JointLims[jointIndex][1],
            Joint0s[jointIndex],
            Joint90s[jointIndex]
        );
    }
}


void loop()
{
    SendArmStatus(ArmStatus);
    Serial.print("x");
    delay(1000);
}




