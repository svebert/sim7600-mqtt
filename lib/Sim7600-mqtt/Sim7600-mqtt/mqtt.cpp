#include "mqtt.h"

namespace SIM7600MQTT
{
    ClMQTTClient::ClMQTTClient(int nTX, int nRX, int nBaudRate, Stream *pLog) :
    m_oSerial(nTX, nRX, nBaudRate, 115200, pLog),
    m_pDbgLog(pLog),
    m_bIsConnected(false)
    { }


    int ClMQTTClient::connect(String sHost, int nPort, String sUsername, String sPassword, const char* szMQTTClientID)
    {
        if(m_oSerial.init() != 0)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to init serial");}
            return -1;
        }

        if(m_oSerial.sendCheckReply("AT+CMQTTSTART", "+CMQTTSTART: 23")) //network is already open
        {
            m_bIsConnected = true;
            return 0;
        }

        String sMsg = "AT+CMQTTACCQ=0,\"";
        sMsg += String(szMQTTClientID);
        sMsg += "\",0";
        bool bRes = m_oSerial.sendCheckReply(sMsg.c_str());
        if(!bRes){
            return -2;
        }

        sMsg = "AT+CMQTTCONNECT=0,\"tcp://" + String(sHost) + ":" + String(nPort) + "\",90,1,\"" + String(sUsername) +"\",\"" + String(sPassword) +"\"";
        bRes = m_oSerial.sendCheckReply(sMsg.c_str());
        m_bIsConnected = bRes;
        if(!bRes)
        {
            return -3;
        }
        return 0;
    }

    int ClMQTTClient::disconnect()
    {
        bool bIsConnected = m_oSerial.sendCheckReply("AT+CMQTTDSIC?", "+CMQTTDISC:0,1");
        if(!bIsConnected){
            m_oSerial.sendCheckReply("AT+CMQTTDSIC=0,60");
        }
        m_oSerial.sendCheckReply("AT+CMQTTREL=0");
        m_oSerial.sendCheckReply("AT+CMQTTSTOP");
        m_bIsConnected = false;
        return 0;
    }

    int ClMQTTClient::publish(String sFeed, String sMessage)
    {

        String sMsg("AT+CMQTTTOPIC=0,");
        sMsg += String(sFeed.length());
        m_oSerial.println(sMsg.c_str());
        m_oSerial.sendCheckReply(sFeed.c_str());

        sMsg = "AT+CMQTTPAYLOAD=0,";
        sMsg += String(sMessage.length());
        m_oSerial.println(sMsg.c_str());
        m_oSerial.sendCheckReply(sMessage.c_str());

        return m_oSerial.sendCheckReply("AT+CMQTTPUB=0,1,60") ? 0 : -1;

    }

    int ClMQTTClient::subscribe(String sFeed)
    {
        // String sMsg("AT+CMQTTSUBTOPIC=0,");
        // sMsg += String(sFeed.length());
        // sMsg += ",1";
        // m_oSerial.println(sMsg.c_str());
        // int nRes = 0;
        // nRes = m_oSerial.sendCheckReply(sFeed.c_str());
        // return nRes;
    }

    int ClMQTTClient::get_subscribe(String* sMsg, int* pnMsg){

    }

    bool ClMQTTClient::isConnected() {return m_bIsConnected;}
}