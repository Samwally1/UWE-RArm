#include <Arduino.h>
#include <ESP32Servo.h>
#include "WifiFunctions.h"
#include "SticksToXYZ.h"
#include "CalibrationFuncs.h"

String ArmMode = "";
String ArmStatus = "Starting";

static unsigned long lastPositionUpdateTime = 0;
static unsigned long lastPositionPrintTime = 0;

StickInput controllerInput = {0, 0, 0, 0, 0, 0, 0, 0};
Servo jointServos[4];

RobotPosition targetPosition = {
    HomeChordsX,
    HomeChordsY,
    HomeChordsZ,
    HeadAngle,
};


void setup(){   

    Serial.begin(9600);

    for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
        jointServos[jointIndex].setPeriodHertz(50);
        jointServos[jointIndex].attach(JointPins[jointIndex], 500, 2400);
        jointServos[jointIndex].write(90);
    }

    if (ConnectToWiFi()) {
        SendArmStatus("Power on: connected to WiFi");
        const float latency = TestLatency();
        delay(1000);
        if (latency >= 0.0f) {
            SendArmStatus("Latency = " + String(latency, 2) + " ms");
        } else {
            SendArmStatus("Latency test failed");
        }
    }
    ArmMode = CheckControllerMode();

    const bool calibrationLoaded = LoadCalibration();
    const bool trainingRequestedAtStartup = GetControllerState(controllerInput)
        && controllerInput.aButton == 1;

    if (ArmMode == "Training" || !calibrationLoaded || trainingRequestedAtStartup) {
        if (trainingRequestedAtStartup) {
            SendArmStatus("Training requested: release A to begin");
            while (controllerInput.aButton == 1) {
                GetControllerState(controllerInput);
                delay(10);
            }
        }

        if (!calibrationLoaded){
            SendArmStatus("No Trianing Config Found ...");
        };

        SendArmStatus("Training Angles");

        const char* operationNames[3] = {
            "Min angle",
            "Max angle",
            "Home angle",
        };

        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            int angleOutputs[4] = {0, 0, 0, 0};

            if(jointIndex == 1){

                jointServos[2].write(0);
            }

            for (int operationIndex = 0; operationIndex < 3; ++operationIndex) {

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

                        lastPositionPrintTime = currentTime;
                    }
                }

                angleOutputs[operationIndex] = static_cast<int>(targetPosition.angle);

                targetPosition.angle = JointHomes[jointIndex];
                jointServos[jointIndex].write(JointHomes[jointIndex]);

                while (controllerInput.aButton == 1) {
                    GetControllerState(controllerInput);
                }
            }

            JointLims[jointIndex][0] = angleOutputs[0];
            JointLims[jointIndex][1] = angleOutputs[1];
            JointHomes[jointIndex] = angleOutputs[2];
        }
    

        Serial.println("Joint calibration:");

        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            const String jointCalibration = "Joint " + String(jointIndex + 1)
                + " | Min: " + String(JointLims[jointIndex][0])
                + " | Max: " + String(JointLims[jointIndex][1])
                + " | Home: " + String(JointHomes[jointIndex]) + " deg";
            SendArmStatus(jointCalibration);
        }

        if (SaveCalibration()) {
            SendArmStatus("Calibration saved");
        } else {
            SendArmStatus("Calibration save failed");
        }
    } else {
        SendArmStatus(CalibrationStatus());

    }
}


void loop()
{
    if (GetControllerState(controllerInput) && controllerInput.bButton == 1) {
        for (int jointIndex = 0; jointIndex < 4; ++jointIndex) {
            jointServos[jointIndex].write(JointHomes[jointIndex]);
        }
        SendArmStatus("B pressed: moving all joints home");
    }

    delay(20);
}




