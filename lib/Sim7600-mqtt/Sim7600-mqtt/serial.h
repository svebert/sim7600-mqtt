#pragma once
#include <Arduino.h>


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
            const unsigned long m_nBaudRate;
            const unsigned long m_nBaudRateInit;
            Stream * m_pDbgLog; //serial for debugging 
            char m_aReplybuffer[BUFFERLEN];
            bool m_bInit{false};
            inline int available(void) {return Serial1.available();}
            inline size_t write(uint8_t x) {return Serial1.write(x);}
            inline int read(void) {return Serial1.read();}
            inline int peek(void) {return Serial1.peek();}
            inline void flush() {Serial1.flush();}
            void flushInput();
            uint8_t readline(uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS, bool multiline = false);
            uint8_t getReply(const char *send, uint16_t timeout = SIM7600MQTT_DEFAULT_TIMEOUT_MS);
            int init();
    };
}