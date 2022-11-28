#pragma once
#include <Arduino.h>

#define RESET_PIN 4

class ClReset{
    public:

    void Init(){
        pinMode(RESET_PIN, OUTPUT);
        digitalWrite(RESET_PIN, HIGH);
    }

    void HardReset(){
        digitalWrite(RESET_PIN, LOW);
    }
};

ClReset * g_pReset = nullptr;
