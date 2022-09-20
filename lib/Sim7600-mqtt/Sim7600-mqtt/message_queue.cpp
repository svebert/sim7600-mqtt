#include "message_queue.h"
#include <Arduino.h>

namespace SIM7600MQTT
{
    
        bool ClMessageQueue::Init(ClMQTTClient* pMQTTClient, const String * pFeeds, Stream * pDbgLog)
        {
            m_pDbgLog = pDbgLog;

            //copy feeds
            for(int nI = 0; nI< MESSAGE_QUEUE_FEED_COUNT; ++nI)
            {
                memset(&(m_aBuffers[nI].m_sFeed[0]), 0, MESSAGE_QUEUE_FEED_LEN);
                memset(&(m_aBuffers[nI].m_stElement[0].m_szMsg[0]), 0, MESSAGE_QUEUE_MSG_LEN*MESSAGE_QUEUE_SIZE);
                memset(&(m_aBuffers[nI].m_aTimestamps[0]), 0, sizeof(unsigned long)*MESSAGE_QUEUE_SIZE);    
                if(pFeeds[nI].length() > MESSAGE_QUEUE_FEED_LEN -1){
                    if(m_pDbgLog){m_pDbgLog->println("Error: feed name is too long");}
                    return false;
                }           
                strlcpy(m_aBuffers[nI].m_sFeed, pFeeds[nI].c_str(), MESSAGE_QUEUE_FEED_LEN);
            }
            
            if(!pMQTTClient)
            {
                return false;
            }
            m_pMQTTClient = pMQTTClient;

            m_nErrorCount = 0;
            m_nPublishCount = 0;
            m_nConnectionError = 0;
            return true;
        }

        bool ClMessageQueue::AddMessageToBuffer(int nFeedIdx, int nFreeBufferIdx, const String& sMsg)
        {
                strlcpy(m_aBuffers[nFeedIdx].m_stElement[nFreeBufferIdx].m_szMsg, sMsg.c_str(), MESSAGE_QUEUE_MSG_LEN);
                m_aBuffers[nFeedIdx].m_stElement[nFreeBufferIdx].m_szMsg[min(sMsg.length(), MESSAGE_QUEUE_MSG_LEN -1)] = 0;
                m_aBuffers[nFeedIdx].m_aTimestamps[nFreeBufferIdx] = millis();
                return true;
        }

        bool ClMessageQueue::Send()
        {
            //connect, send, disconnect(?), free buffers
            if(!m_pMQTTClient){
                return false;
            }
            // if(m_pDbgLog){m_pDbgLog->println("is connected? while...");}
            int nConnection = 0;
            while(!m_pMQTTClient->isConnected() && nConnection < m_nMaxReconnections)
            {
                m_pMQTTClient->connect();
                delay(3000);
                ++nConnection;
            }
            // if(m_pDbgLog){m_pDbgLog->println("is connected?");}
            if(!m_pMQTTClient->isConnected())
            {
                m_nConnectionError = 1;
                return false;
            }
            delay(500);
            //if(m_pDbgLog){m_pDbgLog->println("send stuff");}
            if(!m_bSendJson){
                SendIterative();
            }
            else{
                SendJson();
            }

            if(m_nErrorCount == 0)
            {
                return true;
            }
            else{
                return false;
            }

            if(m_pMQTTClient->disconnect() != 0)
            {
                m_nConnectionError = 2;
                return false;
            }
            
        }

        bool ClMessageQueue::SendIterative(){
            for(size_t nFeed = 0; nFeed < MESSAGE_QUEUE_FEED_COUNT; ++nFeed)
            {
                size_t nFreeBufferIdx = GetFreeBuffer(nFeed);
                //if(m_pDbgLog){m_pDbgLog->println(String(nFeed));}
                for(size_t nBuffer = 0; nBuffer < nFreeBufferIdx; ++nBuffer)
                {
                    m_nPublishCount++;
                    // if(m_pDbgLog){m_pDbgLog->println(String(nBuffer));}
                    // if(m_pDbgLog){m_pDbgLog->println(&m_aBuffers[nFeed].m_sFeed[0]);}
                    // if(m_pDbgLog){m_pDbgLog->println(&m_aBuffers[nFeed].m_stElement[nBuffer].m_szMsg[0]);}
                    if(m_pMQTTClient->publish(&m_aBuffers[nFeed].m_sFeed[0], &m_aBuffers[nFeed].m_stElement[nBuffer].m_szMsg[0]) !=0)
                    {
                        m_nErrorCount++;
                    }
                    delay(250);
                }
            }
            return true;
        }

        bool ClMessageQueue::SendJson(){
            for(size_t nFeed = 0; nFeed < MESSAGE_QUEUE_FEED_COUNT; ++nFeed)
            {
                size_t nFreeBufferIdx = GetFreeBuffer(nFeed);
                String sJsonMsg("[");
                for(size_t nBuffer = 0; nBuffer < nFreeBufferIdx; ++nBuffer)
                {
                    String sElement("{\"value\": ");
                    sElement += String(&m_aBuffers[nFeed].m_stElement[nBuffer].m_szMsg[0]);
                    sElement += String(", \"offset\": ");
                    sElement += String(m_aBuffers[nFeed].m_aTimestamps[nBuffer] - m_aBuffers[nFeed].m_aTimestamps[0]);
                    if(nBuffer < nFreeBufferIdx -1){
                        sElement += "},";
                    }
                    else{
                        sElement += "}";
                    }
                    sJsonMsg += sElement;
                }          
                sJsonMsg += "]";      
                m_nPublishCount++;
                if(m_pMQTTClient->publish(&m_aBuffers[nFeed].m_sFeed[0], sJsonMsg.c_str()) !=0)
                {
                    m_nErrorCount++;
                }
                delay(250);
            }



            return true;
        }

        bool ClMessageQueue::AddMessage(int nFeedIdx, const String& sMsg)
        {

            if(nFeedIdx >= MESSAGE_QUEUE_FEED_COUNT){
                return false;
            }

            m_nErrorCount = 0;
            m_nPublishCount = 0;
            m_nConnectionError = 0;
            size_t nFreeBufferIdx = GetFreeBuffer(nFeedIdx);
            if(nFreeBufferIdx < MESSAGE_QUEUE_SIZE)
            {
                // if(m_pDbgLog){m_pDbgLog->println(String(F("Add to buffer:")) + String(nFreeBufferIdx) +String("/") + String(nFeedIdx));}
                return AddMessageToBuffer(nFeedIdx, nFreeBufferIdx, sMsg);
            }
            else
            {
                // if(m_pDbgLog){m_pDbgLog->println(String(F("Send")));}
                bool bSendSuccess = Send();

                for(int nI = 0; nI< MESSAGE_QUEUE_FEED_COUNT; ++nI)
                {
                    memset(&(m_aBuffers[nI].m_stElement[0].m_szMsg[0]), 0, MESSAGE_QUEUE_MSG_LEN*MESSAGE_QUEUE_SIZE);
                    memset(&(m_aBuffers[nI].m_aTimestamps[0]), 0, sizeof(unsigned long)*MESSAGE_QUEUE_SIZE);                     
                }

                if(!bSendSuccess){
                    return false;
                }
                else
                {
                    return AddMessageToBuffer(nFeedIdx, 0, sMsg);
                }
            }           
        }

        int ClMessageQueue::GetFreeBuffer(int nFeedIdx)
        {
            if(nFeedIdx >= MESSAGE_QUEUE_FEED_COUNT){
                return MESSAGE_QUEUE_SIZE;
            }
            for(size_t nIdx = 0; nIdx < MESSAGE_QUEUE_SIZE; nIdx++)
            {
                if(strlen(&m_aBuffers[nFeedIdx].m_stElement[nIdx].m_szMsg[0]) == 0 )
                {
                    return nIdx;
                }
            }
            return MESSAGE_QUEUE_SIZE;
        }

} // namespace SIM7600MQTT
