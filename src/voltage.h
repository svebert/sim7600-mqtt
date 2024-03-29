#pragma once
#include <Arduino.h>
#include <Stream.h>
#define ANALOG_IN_PIN_1 A1
#define ANALOG_IN_PIN_2 A2
#define ANALOG_IN_PIN_3 A3
#define R1_Ohm 55900.0
#define R2_Ohm 2260.0
#define REFERENCE_VOLTAGE 2.23//4.096
// #define VOLTAGERELAY_PIN_ONOFF 4

class ClVoltageMeasurement{
    public:
    ClVoltageMeasurement(Stream * pDbgLog = nullptr) :
    m_pDbgLog(pDbgLog)
    {
        analogReference(AR_INTERNAL);
    }

    float MeasureVoltage(unsigned int nNr=0){
        int nADCValue;
        pin_size_t nPin;
        switch (nNr)
        {
        case 1:
            nPin = ANALOG_IN_PIN_2;
            break;
        case 2:
            nPin = ANALOG_IN_PIN_3;
            break;  
        case 0:              
        default:
            nPin = ANALOG_IN_PIN_1;

            break;
        }
        for(size_t nI = 0; nI < 10; nI++){
            nADCValue = analogRead(nPin);
        }
        float fADCVoltage = (nADCValue * REFERENCE_VOLTAGE)/1024.0;
        float fInVoltage = fADCVoltage / (R2_Ohm/(R1_Ohm+R2_Ohm));
        if(m_pDbgLog){
            m_pDbgLog->println(String("Voltage(") + String(nNr) + String(") ADCVal=") + String(nADCValue) + String("-->") + String(fInVoltage, 2));
        }
        return fInVoltage;
    }

    // void SwitchVoltageRelayOn(){
    //     if(m_nVoltageRelay == 1 || m_nVoltageRelay == 2){
    //         digitalWrite(VOLTAGERELAY_PIN_ONOFF, HIGH);
    //     }
    // }

    // void SwitchVoltageRelayOff(){
    //     if(m_nVoltageRelay == 0 || m_nVoltageRelay == 2){
    //         digitalWrite(VOLTAGERELAY_PIN_ONOFF, LOW);
    //     }
    // }

    // void SetVoltageRelayState(unsigned char n){
    //     m_nVoltageRelay = n;
    //     if(n == 0){
    //         SwitchVoltageRelayOff();
    //     }
    // }
    // unsigned char GetVoltageRelayState(){
    //     return m_nVoltageRelay;
    // }

    private:
    Stream * m_pDbgLog {nullptr};
    //unsigned char m_nVoltageRelay{2};

};

ClVoltageMeasurement * g_pVoltage = nullptr;

