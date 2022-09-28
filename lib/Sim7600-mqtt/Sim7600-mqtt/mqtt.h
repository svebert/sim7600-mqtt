#pragma once

#include <Stream.h>
#include "serial.h"

namespace SIM7600MQTT
{

class ClMQTTClient
{
    public:
        ClMQTTClient(String sConnection, int nTX, int nRX, unsigned int nBaudRate=19200U, Stream * pDbgLog=nullptr);
        ~ClMQTTClient();
        int connect(unsigned int nRepeatScaler=1);
        int disconnect();
        int publish(const char* szFeed, const char* szMessage);
        // int get_subscribe(const String& sFeed, String& sMsg); //get retained message
        int subscribe_retained(const String& sFeed, String& rsMsg); //get retained message
        bool GetMessage(const String& sFeed, unsigned long& rNumber);
        bool GetMessage(const String& sFeed, String& sMsg);
        bool isConnected();
    private:
        bool flightMode();
        bool ConnectionStatus();
        bool Parse(const String& sIn, String& sOutMsg);
        const String m_sConnection;
        ClATCommandSerial m_oSerial;
        Stream * m_pDbgLog {nullptr};
        bool m_bConnected{true};
};

}