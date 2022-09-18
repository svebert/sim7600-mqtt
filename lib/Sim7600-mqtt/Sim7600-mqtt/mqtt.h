#pragma once

#include <SoftwareSerial.h>
#include "serial.h"

namespace SIM7600MQTT
{

class ClMQTTClient
{
    public:
        ClMQTTClient(String sConnection, int nTX, int nRX, unsigned int nBaudRate=19200U, Stream * pDbgLog=nullptr);
        ~ClMQTTClient();
        int connect();
        int disconnect();
        int publish(const char* szFeed, const char* szMessage);
        int get_subscribe(const String& sFeed, String& sMsg); //get retained message
        bool isConnected();
    private:
        bool ConnectionStatus();
        bool Parse(const String& sIn, String& sOutMsg);
        const String m_sConnection;
        ClATCommandSerial m_oSerial;
        Stream * m_pDbgLog {nullptr};
};

}