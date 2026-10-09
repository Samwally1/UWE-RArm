#include <Arduino.h>

#include "WifiFunctions.h"

void setup() {
    Serial.begin(9600);

    if (ConnectToWiFi()) {
        TestLatency();
        bool x = CheckControllerMode();
    }
}

void loop() {
    if(x == Training)
