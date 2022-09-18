#include "mqtt.h"
#include <Arduino.h>

namespace SIM7600MQTT
{
    ClMQTTClient::ClMQTTClient(String sConnection, int nTX, int nRX, unsigned int nBaudRate, Stream *pLog) :
    m_sConnection(sConnection),
    m_oSerial(nTX, nRX, nBaudRate, 115200U, pLog),
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
            return -1;
        }
        String sReply;
        bool bHaveReply = m_oSerial.getReply("AT+CMQTTDISC?", sReply);
        if(!bHaveReply)
        {
            if(m_pDbgLog){m_pDbgLog->println("Failed to receive reply from AT+CMQTTDSIC?");}
            return -2;
        }   
        if(sReply == F("+CMQTTDISC: 0,0"))
        {
            if(m_pDbgLog){m_pDbgLog->println("!connected!");}      
            return true;
        }
        else if(sReply == F("+CMQTTDISC: 0,1"))
        {
            if(m_pDbgLog){m_pDbgLog->println("!not connected!");}      
        }
        else
        {
            if(m_pDbgLog){m_pDbgLog->println("!dunno!");}      
        }
        return false;
    }

    int ClMQTTClient::connect()
    {
        if(m_oSerial.init() != 0)
        {
            return -1;
        }
        delay(200);
        m_oSerial.sendCheckReply("AT+CMQTTSTART");
        delay(200);
        m_oSerial.sendCheckReply("AT+CMQTTACCQ=0,\"sven-860524\",0"); 
        //if(m_pDbgLog){m_pDbgLog->println(sMsg);}
        m_oSerial.sendCheckReply(m_sConnection.c_str());
        delay(200);
        return 0;
    }

    int ClMQTTClient::disconnect()
    {
        if(m_oSerial.init() != 0)
        {
            return -1;
        }

        m_oSerial.sendCheckReply("AT+CMQTTDISC=0,60");        
        m_oSerial.sendCheckReply("AT+CMQTTREL=0");
        m_oSerial.sendCheckReply("AT+CMQTTSTOP");
        return ConnectionStatus() ? -1 : 0;
    }

    int ClMQTTClient::publish(const char* szFeed, const char* szMessage)
    {
        if(m_pDbgLog){m_pDbgLog->println("--publish");}
        String sMsg(F("AT+CMQTTTOPIC=0,"));
        sMsg += String(strlen(szFeed));
        if(m_pDbgLog){m_pDbgLog->println(szFeed);}
        m_oSerial.sendCheckReply(sMsg.c_str(), ">");
        m_oSerial.sendCheckReply(szFeed);
        sMsg = F("AT+CMQTTPAYLOAD=0,");
        sMsg += String(strlen(szMessage));
        m_oSerial.sendCheckReply(sMsg.c_str(), ">");
        m_oSerial.sendCheckReply(szMessage);

        return m_oSerial.sendCheckReply("AT+CMQTTPUB=0,1,100") ? 0 : -1;
    }

    bool ClMQTTClient::Parse(const String& sIn, String& sOutMsg){
        const String sSearchString(F("+CMQTTRXPAYLOAD: 0,"));
        int nPos = sIn.indexOf(sSearchString);
        if(nPos < 0){
            return false;
        }
        int nPos2 = sIn.indexOf("\n",nPos+sSearchString.length());
        if(nPos2 < 0){
            return false;
        }

        String sMsgLen = sIn.substring(nPos + sSearchString.length(), nPos2);    
        //if(m_pDbgLog){m_pDbgLog->println(">>>" + sMsgLen);}    
        int nMsgLen = atoi(sMsgLen.c_str());
        if(sIn.length() < nPos2 + 1 + nMsgLen){
            return false;
        }
        sOutMsg = sIn.substring(nPos2 + 1, nPos2 + 1 + nMsgLen);
        //if(m_pDbgLog){m_pDbgLog->println(">>>" + sOutMsg);}  
        return true;
    }

    int ClMQTTClient::get_subscribe(const String& sFeed, String& rsMsg){

        delay(250);
        String sATMsg(F("AT+CMQTTSUBTOPIC=0,"));
        sATMsg += String(sFeed.length()) + F(",1");
        //if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        String sATReply;
        m_oSerial.getReply(sATMsg.c_str(), sATReply);
        if(sATReply == F("+CMQTTSUBTOPIC: 0,14"))
        {
            return 0;
        }
        delay(250);
        m_oSerial.sendCheckReply(sFeed.c_str());
        delay(250);
        m_oSerial.sendCheckReply("AT+CMQTTSUB=0");
        delay(250);
        String sFeedGet = sFeed + F("/get");
        sATMsg = F("AT+CMQTTTOPIC=0,");
        sATMsg += String(sFeedGet.length());
        //if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        m_oSerial.getReply(sATMsg.c_str(), sATReply);
        if(sATReply == F("+CMQTTTOPIC: 0,14"))
        {
            return 0;
        }
        delay(250);
        m_oSerial.sendCheckReply(sFeedGet.c_str());

        m_oSerial.sendCheckReply("AT+CMQTTPAYLOAD=0,1",">");
        m_oSerial.sendCheckReply("1");

        m_oSerial.sendCheckReply("AT+CMQTTPUB=0,1,100");
        String sMsgBack;
        bool bHaveMsg = false;
        if(m_oSerial.readlines(sMsgBack, 2000)){
            bHaveMsg = Parse(sMsgBack, rsMsg);
        }
        sATMsg = F("AT+CMQTTUNSUB=0,");
        sATMsg += String(sFeed.length()) + F(",0");
        //if(m_pDbgLog){m_pDbgLog->println(sATMsg);}
        m_oSerial.sendCheckReply(sATMsg.c_str(), ">");
        m_oSerial.sendCheckReply(sFeed.c_str());
        return bHaveMsg ? 0 : -1;
    }

    bool ClMQTTClient::isConnected() {return ConnectionStatus();}
}