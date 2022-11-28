#include "serial.h"
#include <Arduino.h>

namespace SIM7600MQTT
{

    ClATCommandSerial::ClATCommandSerial(int nTX, int nRX, unsigned long nBaudRate, unsigned long nBaudRateInit, Stream *pLog) :
#ifdef MICRO
        SERIAL(nRX, nTX),
#endif
        m_nBaudRate(nBaudRate),
        m_nBaudRateInit(nBaudRateInit),
        m_pDbgLog(pLog),
        m_bInit(false)
    {  }


    bool ClATCommandSerial::HaveBaudRate(unsigned long nBaudRate){
        SERIAL.begin(nBaudRate);
        delay(250);
        sendCheckReply("ATE0");
        delay(100);
        bool bOK = sendCheckReply("AT");
        SERIAL.begin(m_nBaudRate);
        delay(250);
        return bOK;
    }

    bool ClATCommandSerial::SetBaudRate(unsigned long nOldBaudRate, unsigned long nNewBaudRate){
        SERIAL.begin(nOldBaudRate);
        delay(250);
        sendCheckReply("ATE0");
        delay(100);
        String sMsg(F("AT+IPR="));
        sMsg += String(nNewBaudRate);
        sendCheckReply(sMsg.c_str(), "OK");
        delay(250);
        SERIAL.begin(nNewBaudRate);
        delay(250);
        return sendCheckReply("AT");
    }

    int ClATCommandSerial::init()
    {
        if(m_bInit)
        {
            return 0;
        }

        // SERIAL.begin(m_nBaudRateInit);
        // sendCheckReply("ATE0", "OK");
        // delay(100);
        // String sMsg(F("AT+IPRX="));
        // sMsg += String(m_nBaudRate);
        // delay(250);
        // sendCheckReply(sMsg, "OK");

        m_bInit = true; //to prevent infinit loop
        if(HaveBaudRate(m_nBaudRateInit))
        {
            SetBaudRate(m_nBaudRateInit, m_nBaudRate);
            if(m_pDbgLog){ m_pDbgLog->println(F("Set Baud -> Ok"));}
            m_bInit = true;
            return 0;
        }
        else if (HaveBaudRate(m_nBaudRate))
        {
            //good
            if(m_pDbgLog){ m_pDbgLog->println(F("Keep Baud -> Ok"));}
            m_bInit = true;
            return 0;
        }
        else
        {
            if(m_pDbgLog){ m_pDbgLog->println(F("Baud Failed! Power off?"));}

    
            // delay(1000);
  
            SERIAL.begin(m_nBaudRate);
            delay(250);
            sendCheckReply("AT+CRESET", "OK", 3000);
            bool bHaveReboot = false;
            for(unsigned int i = 0; i < 30; ++i){
                if(sendCheckReply("ATE0", "OK")){bHaveReboot=true;break;}
                delay(2000);
            }
            if(!bHaveReboot){
                if(m_pDbgLog){ m_pDbgLog->println(F("Try 115200"));}
                SERIAL.begin(m_nBaudRateInit);
                delay(250);
                sendCheckReply("AT+CRESET", "OK", 3000);
                for(unsigned int i = 0; i < 30; ++i){
                    if(sendCheckReply("ATE0", "OK")){bHaveReboot=true;break;}
                    delay(2000);
                }
                SetBaudRate(m_nBaudRateInit, m_nBaudRate);
            }

            for(size_t nCnt = 0; nCnt < 30; ++nCnt){
                String sMsg;
                sendCheckReply("ATE0", "OK");
                bool bHaveReply = getReply("AT+CMQTTDISC?", sMsg);
                if(bHaveReply && sMsg.startsWith("+CMQTTDISC:")){
                    m_bInit = true;
                    if(m_pDbgLog){ m_pDbgLog->println(F("power on"));}
                    return 0; //already powered on
                }
                delay(2000);
            }

            if(m_pDbgLog){ m_pDbgLog->println(F("power off -- Failed!"));}
            m_bInit = false;
            return -1; //power off?
        }
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

            while(SERIAL.available()) {
            char c = SERIAL.read();
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

        SERIAL.println(send);
        uint8_t l = readline(timeout);
        if(m_pDbgLog){m_pDbgLog->print(F("\t<--- "));m_pDbgLog->println(m_aReplybuffer);}

        return l;
    }

    bool ClATCommandSerial::readlines(String & rsReply, uint16_t timeout) 
    {
        if(init() != 0){return false;}
        uint16_t replyidx = readline(timeout, true);

        if (replyidx > 0)
        {
            rsReply = String(m_aReplybuffer);
        }
        return replyidx > 0 ? 1 : 0;
    }

    bool ClATCommandSerial::getReply(const char *send, String & rsReply, uint16_t timeout) 
    {
        if(init() != 0){return false;}
        uint8_t l = getReply(send, timeout);
        if (l > 0)
        {
            rsReply = String(m_aReplybuffer);
        }

        return l > 0 ? 1 : 0;
    }

    bool ClATCommandSerial::sendCheckReply(const char *send, const char *reply, uint16_t timeout)
    {
        if(init() != 0){return false;}
        if (! getReply(send, timeout) )
            return false;
        return (strcmp(m_aReplybuffer, reply) == 0);
    }
    

    void ClATCommandSerial::println(const char* szMsg, int nDelay)
    {
        init();
        SERIAL.println(szMsg);
        delay(nDelay);
    }

    void ClATCommandSerial::println(String sMsg, int nDelay)
    {
        init();
        SERIAL.println(sMsg);
        delay(nDelay);
    }
}
