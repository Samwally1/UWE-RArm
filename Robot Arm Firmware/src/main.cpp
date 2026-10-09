#include <Arduino.h>

#include "WifiFunctions.h"

void setup() {
    Serial.begin(9600);
    ConnectToWiFi();
    TestLatency();
}

void loop() {
}
