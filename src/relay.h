#pragma once
#include <Arduino.h>

#define RELAY_PIN_ONOFF 5

class ClRelay{
    public:

    void Init(){
        pinMode(RELAY_PIN_ONOFF, OUTPUT);
    }

    void On(){
        digitalWrite(RELAY_PIN_ONOFF, HIGH);
        m_nCheck=0;
        m_bState = true;
    }
    void Off(){
        digitalWrite(RELAY_PIN_ONOFF, LOW);
        m_nCheck=0;
        m_bState = false;
    }

    void Check(unsigned long nMaxChecks=60){
        if(++m_nCheck > nMaxChecks){
            Off();
        }
    }

    bool Status(){
        return m_bState;
    }
    private:
        unsigned long m_nCheck{0};
        bool m_bState{false};

};

ClRelay * g_pPowerRelay = nullptr;
