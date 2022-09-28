#pragma once

#ifdef MICRO
#include <SoftwareSerial.h>
#define SERIAL m_oSerial
#else
#include <Arduino.h>
#define SERIAL Serial1
#endif

#define SIM7600MQTT_DEFAULT_TIMEOUT_MS 500
#define REPLY_OK "OK"
#define BUFFERLEN 127

namespace SIM7600MQTT
{
    class ClATCommandSerial
    {
        public:
            ClATCommandSerial(int nTX, int nRX, unsigned long nBaudRate, unsigned long nBaudRateInit=115200U, Stream *pLog=nullptr);      
            bool sendCheckReply(const char* send, const char* reply = REPLY_OK, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS); 
            bool getReply(const char *send, String & rsReply, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
            void println(const char* send, int nDelay=100);  
            void println(String send, int nDelay=100);  
            bool readlines(String & rsReply, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
            void DeInit(){m_bInit=false;}
            bool HaveBaudRate(unsigned long nBaudRate);
            bool SetBaudRate(unsigned long nOldBaudRate, unsigned long nNewBaudRate);
        private:
            #ifdef MICRO
            SoftwareSerial SERIAL; //main serial
            #endif
            const unsigned long m_nBaudRate;
            const unsigned long m_nBaudRateInit;
            Stream * m_pDbgLog; //serial for debugging 
            char m_aReplybuffer[BUFFERLEN];
            bool m_bInit{false};

            inline int available(void) {return SERIAL.available();}
            inline size_t write(uint8_t x) {return SERIAL.write(x);}
            inline int read(void) {return SERIAL.read();}
            inline int peek(void) {return SERIAL.peek();}
            inline void flush() {SERIAL.flush();}
            void flushInput();
            uint8_t readline(uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS, bool multiline = false);
            uint8_t getReply(const char *send, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
            int init();
    };
}