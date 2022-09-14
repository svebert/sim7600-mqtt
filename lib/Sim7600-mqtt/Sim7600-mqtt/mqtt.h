#pragma once

#include <USBAPI.h>
#include "serial.h"

namespace SIM7600MQTT
{

class ClMQTTClient
{
    public:
        ClMQTTClient(int nTX, int nRX, int nBaudRate=19200, Stream * pDbgLog=nullptr);
        ~ClMQTTClient();
        int connect(String sHost, int nPort, String sUsername, String sPassword, const char* szMQTTClientID="sven-test-mqtt-id");
        int disconnect();
        int publish(String sFeed, String sMessage);
        int get_subscribe(String sFeed, String& sMsg); //get retained message
        bool isConnected();
    private:
        bool ConnectionStatus();
        ClATCommandSerial m_oSerial;
        Stream * m_pDbgLog {nullptr};
};

}