#include "mqtt.h"

namespace SIM7600MQTT
{
    ClMQTTClient::ClMQTTClient(int nTX, int nRX, int nBaudRate, Stream *pLog) :
    m_oSerial(nTX, nRX, nBaudRate, 115200, pLog),
    m_pDbgLog(pLog)
    { 

    }

    ClMQTTClient::~ClMQTTClient()
    {
        disconnect();
    }

    bool ClMQTTClient::ConnectionStatus()
    {
        if(m_oSerial.init() != 0)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to init serial");}
            return -1;
        }
        String sReply;
        bool bHaveReply = m_oSerial.getReply("AT+CMQTTDISC?", sReply);
        if(!bHaveReply)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to receive reply from AT+CMQTTDSIC?");}
            return -2;
        }   
        if(sReply == "+CMQTTDISC: 0,0")
        {
            if(m_pDbgLog){m_pDbgLog->println("!connected!");}      
            return true;
        }
        else if(sReply == "+CMQTTDISC: 0,1")
        {
            if(m_pDbgLog){m_pDbgLog->println("!not connected!");}      
        }
        else
        {
            if(m_pDbgLog){m_pDbgLog->println("!dunno!");}      
        }
        return false;
    }

    int ClMQTTClient::connect(String sHost, int nPort, String sUsername, String sPassword, const char* szMQTTClientID)
    {
        if(m_oSerial.init() != 0)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to init serial");}
            return -1;
        }

        m_oSerial.sendCheckReply("AT+CMQTTSTART");

        String sMsg = "AT+CMQTTACCQ=0,\"";
        sMsg += String(szMQTTClientID);
        sMsg += "\",0";
        m_oSerial.sendCheckReply(sMsg.c_str());

        sMsg = "AT+CMQTTCONNECT=0,\"tcp://" + String(sHost) + ":" + String(nPort) + "\",90,1,\"" + String(sUsername) +"\",\"" + String(sPassword) +"\"";
        m_oSerial.sendCheckReply(sMsg.c_str());
        return ConnectionStatus() ? 0 : -2;
    }

    int ClMQTTClient::disconnect()
    {
        if(m_oSerial.init() != 0)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to init serial");}
            return -1;
        }

        m_oSerial.sendCheckReply("AT+CMQTTDISC=0,60");        
        m_oSerial.sendCheckReply("AT+CMQTTREL=0");
        m_oSerial.sendCheckReply("AT+CMQTTSTOP");
        return ConnectionStatus() ? -1 : 0;
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

    int ClMQTTClient::get_subscribe(String sFeed, String& rsMsg){


        String sATMsg("AT+CMQTTSUBTOPIC=0,");
        sATMsg += String(sFeed.length()) + ",1";
        if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        m_oSerial.println(sATMsg.c_str());
        m_oSerial.sendCheckReply((sFeed) .c_str());
        m_oSerial.sendCheckReply("AT+CMQTTSUB=0");

        delay(1500);
        String sFeedGet = sFeed + "/get";
        sATMsg = "AT+CMQTTTOPIC=0,";
        sATMsg += String(sFeedGet.length());
        if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        m_oSerial.println(sATMsg.c_str());
        m_oSerial.sendCheckReply((sFeedGet).c_str());

        m_oSerial.println("AT+CMQTTPAYLOAD=0,1");
        m_oSerial.sendCheckReply("1");

        m_oSerial.println("AT+CMQTTPUB=0,1,60");
        String sMsgBack;
        for(int i=0; i<5; i++)
        {
            m_oSerial.readline(sMsgBack);
            if(m_pDbgLog){m_pDbgLog->println(sMsgBack);}
        }

        sATMsg = "AT+CMQTTUNSUB=0,";
        sATMsg += String(sFeed.length()) + ",0";
        if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        m_oSerial.println(sATMsg.c_str());
        m_oSerial.sendCheckReply(sFeed.c_str());
        return 0;
    }

    bool ClMQTTClient::isConnected() {return ConnectionStatus();}
}