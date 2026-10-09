#include <Arduino.h>

#include "WifiFunctions.h"

void setup() {
    Serial.begin(9600);

    if (ConnectToWiFi()) {
        TestLatency();
        CheckControllerMode();
    }
}

void loop() {
}
