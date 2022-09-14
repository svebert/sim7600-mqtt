#pragma once

#include <SoftwareSerial.h>

#define SIM7600MQTT_DEFAULT_TIMEOUT_MS 500
#define REPLY_OK "OK"

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
            bool readline(String & rsReply, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
            int init();
        private:
            
            SoftwareSerial m_oSerial; //main serial
            const unsigned long m_nBaudRate;
            const unsigned long m_nBaudRateInit;
            Stream * m_pDbgLog; //serial for debugging 
            char m_aReplybuffer[255];
            bool m_bInit;

            inline int available(void) {return m_oSerial.available();}
            inline size_t write(uint8_t x) {return m_oSerial.write(x);}
            inline int read(void) {return m_oSerial.read();}
            inline int peek(void) {return m_oSerial.peek();}
            inline void flush() {m_oSerial.flush();}
            void flushInput();
            uint8_t readline(uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS, bool multiline = false);
            uint8_t getReply(const char *send, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
    };
}