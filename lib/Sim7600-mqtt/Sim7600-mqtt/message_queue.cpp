#include "message_queue.h"
#include <Arduino.h>

namespace SIM7600MQTT
{    
        bool ClMessageQueue::Init(ClMQTTClient* pMQTTClient, const String * pFeeds, unsigned int nFeeds, Stream * pDbgLog)
        {
            m_pDbgLog = pDbgLog;
            DeInit();           
            //construct buffers
            m_nBufferCount = nFeeds;
            m_pBuffers = new StBuffer[m_nBufferCount];
            for(unsigned int nI = 0; nI< m_nBufferCount; ++nI)
            {
                m_pBuffers[nI].clear(true);
                if(pFeeds[nI].length() > MESSAGE_QUEUE_FEED_LEN -1){
                    if(m_pDbgLog){m_pDbgLog->println("Error: feed name is too long");}
                    return false;
                }           
                strlcpy(m_pBuffers[nI].m_sFeed, pFeeds[nI].c_str(), MESSAGE_QUEUE_FEED_LEN);
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

        void ClMessageQueue::DeInit(){
            if(m_pBuffers){
               delete[] m_pBuffers;
               m_nBufferCount = 0;
            }
        }

        bool ClMessageQueue::AddMessageToBuffer(unsigned int nFeedIdx, const String& sMsg, unsigned long nTimestamp)
        {
                m_pBuffers[nFeedIdx].m_oData[m_pBuffers[nFeedIdx].m_nBufferIdx] = sMsg;
                m_pBuffers[nFeedIdx].m_aTimestamps[m_pBuffers[nFeedIdx].m_nBufferIdx] = nTimestamp;
                m_pBuffers[nFeedIdx].m_nBufferIdx++;
                return true;
        }

        bool ClMessageQueue::Send(bool bDisconnectWhenSendFinished)
        {
            //connect, send, disconnect(?), free buffers
            if(!m_pMQTTClient){
                return false;
            }

            unsigned int nConnection = 0;
            unsigned int nRepeatScaler = 1;
            while(!m_pMQTTClient->isConnected() && nConnection < m_nMaxReconnections)
            {
                m_pMQTTClient->connect(nRepeatScaler);
                delay(3000);
                ++nConnection;
                nRepeatScaler += 2;
            }

            if(!m_bSendJson){
                SendIterative();
            }
            else{
                SendJson();
            }

            if(bDisconnectWhenSendFinished)
            {
                if(m_pDbgLog){m_pDbgLog->println("MsgQueue -> disconnect");}
                if(m_pMQTTClient->disconnect() != 0){
                    m_nConnectionError = 2;
                    return false;
                }
            }

            if(m_nErrorCount == 0){
                return true;
            }
            else{
                return false;
            }
        }

        bool ClMessageQueue::SendIterative(){
            for(size_t nFeed = 0; nFeed < m_nBufferCount; ++nFeed)
            {
                for(size_t nBuffer = 0; nBuffer < m_pBuffers[nFeed].m_nBufferIdx; ++nBuffer)
                {
                    m_nPublishCount++;
                    if(m_pMQTTClient->publish(&m_pBuffers[nFeed].m_sFeed[0], m_pBuffers[nFeed].m_oData[nBuffer].c_str()) !=0)
                    {
                        m_nErrorCount++;
                    }
                    delay(250);
                }
            }
            return true;
        }

        bool ClMessageQueue::SendJson(){
            for(size_t nFeed = 0; nFeed < m_nBufferCount; ++nFeed)
            {
                if(m_pBuffers[nFeed].m_nBufferIdx > 0){
                    String sJsonMsg("[");
                    unsigned long nTSend = m_pBuffers[nFeed].m_aTimestamps[m_pBuffers[nFeed].m_nBufferIdx - 1];
                    for(size_t nBuffer = 0; nBuffer < m_pBuffers[nFeed].m_nBufferIdx; ++nBuffer)
                    {
                        String sElement("{\"value\": ");
                        sElement += m_pBuffers[nFeed].m_oData[nBuffer];
                        sElement += String(", \"offset\": ");
                        sElement += String( - static_cast<long>((nTSend - m_pBuffers[nFeed].m_aTimestamps[nBuffer])*100));
                        if(nBuffer < m_pBuffers[nFeed].m_nBufferIdx -1){
                            sElement += "},";
                        }
                        else{
                            sElement += "}";
                        }
                        sJsonMsg += sElement;
                    }          
                    sJsonMsg += "]";      
                    m_nPublishCount++;
                    int nErr = 0;
                    nErr = m_pMQTTClient->publish(&(m_pBuffers[nFeed].m_sFeed[0]), sJsonMsg.c_str());
                    unsigned long nDelay = m_pBuffers[nFeed].m_nBufferIdx*150;
                    for(size_t nResendCount = 0; nResendCount < 4; ++nResendCount)
                    {
                        if(nErr == -1){
                            //try again
                            delay(nDelay + nResendCount*500);
                            nErr = m_pMQTTClient->publish(&(m_pBuffers[nFeed].m_sFeed[0]), sJsonMsg.c_str());
                        }
                        else{
                            break;
                        }
                    }
                    if(nErr!=0){

                        m_nErrorCount++;
                    }
                    delay(1000);
                }
            }

            return true;
        }

        bool ClMessageQueue::AddMessage(unsigned int nFeedIdx, const String& sMsg, unsigned long nTimestamp, bool bDisconnectWhenSendFinished)
        {

            if(nFeedIdx >= m_nBufferCount){
                return false;
            }

            m_nErrorCount = 0;
            m_nPublishCount = 0;
            m_nConnectionError = 0;
            if(m_pBuffers[nFeedIdx].m_nBufferIdx < MESSAGE_MAX_QUEUE_SIZE)
            {
                return AddMessageToBuffer(nFeedIdx, sMsg, nTimestamp);
            }
            else
            {
                bool bSendSuccess = Send(bDisconnectWhenSendFinished);

                for(unsigned int nI = 0; nI< m_nBufferCount; ++nI)
                {
                    m_pBuffers[nI].clear();             
                }
                bool bAddMessage = AddMessageToBuffer(nFeedIdx, sMsg, nTimestamp);
                return bAddMessage && bSendSuccess;                
            }           
        }

} // namespace SIM7600MQTT
