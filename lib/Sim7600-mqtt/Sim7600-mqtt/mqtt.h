#pragma once

#include <USBAPI.h>
#include "serial.h"

namespace SIM7600MQTT
{

class ClMQTTClient
{
    public:
        ClMQTTClient(int nTX, int nRX, int nBaudRate=19200, Stream * pDbgLog=nullptr);
        int connect(String sHost, int nPort, String sUsername, String sPassword, const char* szMQTTClientID="sven-test-mqtt-id");
        int disconnect();
        int publish(String sFeed, String sMessage);
        int subscribe(String sFeed);
        int get_subscribe(String* sMsg, int* pnMsg);
        bool isConnected();
    private:
        Stream * m_pDbgLog {nullptr};
        ClATCommandSerial m_oSerial;
        bool m_bIsConnected{false};
};

}