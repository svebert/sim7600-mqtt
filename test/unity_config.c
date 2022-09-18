
#include "unity_config.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

void unityOutputStart() {}

void unityOutputChar(char c) {
   Serial.print(c);
}

void unityOutputFlush() {}

void unityOutputComplete() {}