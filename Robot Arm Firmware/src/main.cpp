#include "RobotMovement.h"
#include "StartupProcess.h"

void setup(){   
    StartupProcess();
}


void loop()
{
    const RobotPosition position1 = {-100.0f, 200.0f, 100.0f, 0.0f};
    const RobotPosition position2 = {0.0f, 200.0f, 100.0f, 0.0f};
    const RobotPosition position3 = {0.0f, 200.0f, 300.0f, 0.0f};
    const RobotPosition position4 = {0.0f, 200.0f, 100.0f, 0.0f};
    const RobotPosition position5 = {100.0f, 200.0f, 100.0f, 0.0f};

    constexpr float speedMmPerSecond = 50;

    static size_t nextPositionIndex = 0;
    static bool hasPreviousPosition = false;
    static RobotPosition previousPosition;

    if (!hasPreviousPosition) {
        previousPosition = ForwardKinTransform(targetAngles);
        hasPreviousPosition = true;
        Serial.printf(
            "Starting from X: %.1f, Y: %.1f, Z: %.1f, Angle: %.1f\n",
            previousPosition.x,
            previousPosition.y,
            previousPosition.z,
            previousPosition.angle
        );
    }

    const RobotPosition positions[] = {
        position1, position2, position3, position4, position5
    };
    constexpr size_t positionCount = sizeof(positions) / sizeof(positions[0]);
    const RobotPosition& nextPosition = positions[nextPositionIndex];
    const int positionNumber = static_cast<int>(nextPositionIndex) + 1;

    Serial.printf(
        "Moving to position %d | X: %.1f, Y: %.1f, Z: %.1f, Angle: %.1f\n",
        positionNumber,
        nextPosition.x,
        nextPosition.y,
        nextPosition.z,
        nextPosition.angle
    );
    if (CommandJointAnglesLinear(
            nextPosition, previousPosition, speedMmPerSecond
        )) {
        Serial.printf("Reached position %d\n", positionNumber);
        previousPosition = nextPosition;
        nextPositionIndex = (nextPositionIndex + 1) % positionCount;
        delay(1000);
    } else {
        previousPosition = ForwardKinTransform(targetAngles);
        Serial.printf("Position %d was not reached; retrying in 2 seconds\n",
                      positionNumber);
        delay(2000);
    }
}
