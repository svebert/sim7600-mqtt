#pragma once

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

class ClBME680Wrapper 
{
    public:
        ClBME680Wrapper(){}
        ~ClBME680Wrapper(){
                deinit();
        }
        bool init(){
            if(!deinit()){
                return false;}
            m_pBME680 = new Adafruit_BME680();
            if(!m_pBME680->begin()){
                return false;
            }
            m_pBME680->setTemperatureOversampling(BME680_OS_8X);
            m_pBME680->setHumidityOversampling(BME680_OS_2X);
            m_pBME680->setPressureOversampling(BME680_OS_4X);
            m_pBME680->setIIRFilterSize(BME680_FILTER_SIZE_3);
            //m_pBME680->setGasHeater(320, 150); // 320*C for 150 ms
            return true;
        }

        bool deinit(){
            if(m_pBME680){ delete m_pBME680;}
            m_pBME680 = nullptr;
            return true;
        }
        bool performReading(){
            if(m_pBME680)
                return m_pBME680->performReading();
            return false;
        }
        float temperature(){
            if(m_pBME680)
                return m_pBME680->temperature;
            return -1000.0f;
        }
        float pressure(){
            if(m_pBME680)
                return static_cast<float>(m_pBME680->pressure)/100.0f;
            return -1.0f;
        }
        float humidity(){
            if(m_pBME680)
                return m_pBME680->humidity;
            return -1.0f;
        }
        // float gas_resistance(){
        //     if(m_pBME680)
        //         return static_cast<float>(m_pBME680->gas_resistance/1000);
        //     return -1.0f;
        // }
    private: 
        Adafruit_BME680 * m_pBME680 = nullptr;
};

ClBME680Wrapper * g_pBME680{nullptr}; 
