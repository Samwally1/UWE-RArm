#include "RobotMovement.h"
#include "StartupProcess.h"

void setup(){   
    StartupProcess();
}


void loop()
{
    const RobotPosition position1 = {-100.0f, 200.0f, 100.0f, 0.0f};
    const RobotPosition position2 = {100.0f, 200.0f, 100.0f, 0.0f};
    constexpr float speedMmPerSecond = 5;

    static bool moveToPosition1 = true;
    static bool hasPreviousPosition = false;
    static RobotPosition previousPosition;

    if (!hasPreviousPosition) {
        previousPosition = ForwardKinTransform(targetAngles);
        hasPreviousPosition = true;
        Serial.printf(
            "Starting from X: %.1f, Y: %.1f, Z: %.1f, Angle: %.1f\\n",
            previousPosition.x,
            previousPosition.y,
            previousPosition.z,
            previousPosition.angle
        );
    }

    const RobotPosition& nextPosition = moveToPosition1
        ? position1
        : position2;
    const int positionNumber = moveToPosition1 ? 1 : 2;

    Serial.printf(
        "Moving to position %d | X: %.1f, Y: %.1f, Z: %.1f, Angle: %.1f\\n",
        positionNumber,
        nextPosition.x,
        nextPosition.y,
        nextPosition.z,
        nextPosition.angle
    );
    if (CommandJointAnglesLinear(
            nextPosition, previousPosition, speedMmPerSecond
        )) {
        Serial.printf("Reached position %d\\n", positionNumber);
        previousPosition = nextPosition;
        moveToPosition1 = !moveToPosition1;
        delay(1000);
    } else {
        Serial.printf("Position %d was not reached; retrying in 2 seconds\\n",
                      positionNumber);
        delay(2000);
    }
}
