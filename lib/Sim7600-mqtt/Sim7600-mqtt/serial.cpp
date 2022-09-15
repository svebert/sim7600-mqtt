#include "serial.h"
#include <Arduino.h>

namespace SIM7600MQTT
{

    ClATCommandSerial::ClATCommandSerial(int nTX, int nRX, unsigned long nBaudRate, unsigned long nBaudRateInit=115200U, Stream *pLog) :
        m_oSerial(nRX, nTX),
        m_nBaudRate(nBaudRate),
        m_nBaudRateInit(nBaudRateInit),
        m_pDbgLog(pLog),
        m_bInit(false)
    {  }

    int ClATCommandSerial::init()
    {
            if(m_bInit)
            {
                return 0;
            }

            delay(2000);
            m_oSerial.begin(m_nBaudRateInit);
            delay(500);
            sendCheckReply("ATE0"); //disable echo
            delay(100);
            if(!sendCheckReply("AT")){
                m_oSerial.begin(m_nBaudRate);
                delay(500);
                sendCheckReply("ATE0"); //disable echo
                delay(100);
                if(!sendCheckReply("AT")){
                    if(m_pDbgLog){ m_pDbgLog->println("Failed Set BaudRate 1");}
                    return -1;
                }
                else
                {         
                    m_bInit = true;       
                    return 0;
                }
            }

            String sMsg("AT+IPR=");
            sMsg += String(m_nBaudRate);
            sendCheckReply(sMsg.c_str());
            delay(100);
            m_oSerial.begin(m_nBaudRate);
            delay(500);
            if(!sendCheckReply("AT")){
                if(m_pDbgLog){ m_pDbgLog->println("Failed Set BaudRate 2");}
                return -3;
            }

            m_bInit = true;
            return 0;    
    }

    void ClATCommandSerial::flushInput() {
        // Read all available serial input to flush pending data.
        uint16_t timeoutloop = 0;
        while (timeoutloop++ < 40) {
            while(available()) {
                read();
                timeoutloop = 0;  // If char was received reset the timer
            }
            delay(1);
        }
    }

    uint8_t ClATCommandSerial::readline(uint16_t timeout, bool multiline) 
    {
        uint16_t replyidx = 0;

        while (timeout--) 
        {
            if (replyidx >= BUFFERLEN -1) {
            //DEBUG_PRINTLN(F("SPACE"));
            break;
            }

            while(m_oSerial.available()) {
            char c = m_oSerial.read();
            if (c == '\r') continue;
            if (c == 0xA) {
                if (replyidx == 0)   // the first 0x0A is ignored
                continue;

                if (!multiline) {
                timeout = 0;         // the second 0x0A is the end of the line
                break;
                }
            }
            m_aReplybuffer[replyidx] = c;
            //DEBUG_PRINT(c, HEX); DEBUG_PRINT("#"); DEBUG_PRINTLN(c);

            if (++replyidx >= BUFFERLEN -1)
                break;
            }

            if (timeout == 0) {
            //DEBUG_PRINTLN(F("TIMEOUT"));
            break;
            }
            delay(1);
        }
        m_aReplybuffer[replyidx] = 0;  // null term
        return replyidx;
    }

    uint8_t ClATCommandSerial::getReply(const char *send, uint16_t timeout) 
    {
        flushInput();
        if(m_pDbgLog){m_pDbgLog->print(F("\t---> "));m_pDbgLog->println(send);}

        m_oSerial.println(send);
        uint8_t l = readline(timeout);
        if(m_pDbgLog){m_pDbgLog->print(F("\t<--- "));m_pDbgLog->println(m_aReplybuffer);}

        return l;
    }

    bool ClATCommandSerial::readlines(String & rsReply, uint16_t timeout) 
    {
        uint16_t replyidx = readline(timeout, true);

        if (replyidx > 0)
        {
            rsReply = String(m_aReplybuffer);
        }
        return replyidx > 0 ? 1 : 0;
    }

    bool ClATCommandSerial::getReply(const char *send, String & rsReply, uint16_t timeout) 
    {
        uint8_t l = getReply(send, timeout);
        if (l > 0)
        {
            rsReply = String(m_aReplybuffer);
        }

        return l > 0 ? 1 : 0;
    }

    bool ClATCommandSerial::sendCheckReply(const char *send, const char *reply, uint16_t timeout)
    {
        if (! getReply(send, timeout) )
            return false;
        return (strcmp(m_aReplybuffer, reply) == 0);
    }
    

    void ClATCommandSerial::println(const char* szMsg, int nDelay)
    {
        m_oSerial.println(szMsg);
        delay(nDelay);
    }

    void ClATCommandSerial::println(String sMsg, int nDelay)
    {
        m_oSerial.println(sMsg);
        delay(nDelay);
    }
}