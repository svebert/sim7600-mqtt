#pragma once

#include <Stream.h>
#include "serial.h"

namespace SIM7600MQTT
{

class ClMQTTClientI 
{
    public:
        virtual int connect(unsigned int nRepeatScaler=1) = 0;
        virtual int disconnect() = 0;
        virtual int publish(const char* szFeed, const char* szMessage) = 0;
        virtual int subscribe_retained(const String& sFeed, String& rsMsg) = 0; //get retained message
        virtual bool GetMessage(const String& sFeed, unsigned long& rNumber) = 0;
        virtual bool GetMessage(const String& sFeed, String& sMsg) = 0;
        virtual bool isConnected() = 0;
        virtual void reset() = 0;
        virtual int read_gps(String &rsGPS) = 0;
};


class ClMQTTClient : public ClMQTTClientI
{
    public:
        ClMQTTClient(String sConnection, int nTX, int nRX, unsigned int nBaudRate=19200U, Stream * pDbgLog=nullptr, String sAPN="");
        ~ClMQTTClient();
        int connect(unsigned int nRepeatScaler=1);
        int disconnect();
        int publish(const char* szFeed, const char* szMessage);
        // int get_subscribe(const String& sFeed, String& sMsg); //get retained message
        int subscribe_retained(const String& sFeed, String& rsMsg); //get retained message
        bool GetMessage(const String& sFeed, unsigned long& rNumber);
        bool GetMessage(const String& sFeed, String& sMsg);
        bool isConnected();
        void reset();
        int read_gps(String &rsGPS);
    private:
        bool flightMode();
        bool ConnectionStatus();
        bool Parse(const String& sIn, String& sOutMsg);
        bool CheckAPN();
        bool SetAPN();
        const String m_sConnection;
        ClATCommandSerial m_oSerial;
        Stream * m_pDbgLog {nullptr};
        bool m_bConnected{true};
        String m_sAPN;
};


class ClMQTTClientDummy : public ClMQTTClientI
{
    public:
        ClMQTTClientDummy();
        ~ClMQTTClientDummy();
        int connect(unsigned int nRepeatScaler=1);
        int disconnect();
        int publish(const char* szFeed, const char* szMessage);
        int subscribe_retained(const String& sFeed, String& rsMsg); //get retained message
        bool GetMessage(const String& sFeed, unsigned long& rNumber);
        bool GetMessage(const String& sFeed, String& sMsg);
        bool isConnected();
        void reset();
        int read_gps(String &rsGPS);
};

}