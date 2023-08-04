#pragma once
#include "mqtt.h"
#include <Stream.h>

#ifndef MESSAGE_MAX_QUEUE_SIZE
#define MESSAGE_MAX_QUEUE_SIZE 6
#endif
#ifndef MESSAGE_QUEUE_FEED_LEN
#define MESSAGE_QUEUE_FEED_LEN 25
#endif
namespace SIM7600MQTT
{
    typedef String StMsgBuffer;

    struct StBuffer
    {
        void clear(bool bClearFeedName=false)
        {
            if(bClearFeedName){
                memset(&(m_sFeed[0]), 0, MESSAGE_QUEUE_FEED_LEN);
            }
            
            for(unsigned int nIdx=0; nIdx < MESSAGE_MAX_QUEUE_SIZE; ++nIdx){
                m_oData[nIdx] = "";
                m_aTimestamps[nIdx]=0;
            }
            m_nBufferIdx = 0;
        }
        char m_sFeed[MESSAGE_QUEUE_FEED_LEN];
        StMsgBuffer m_oData[MESSAGE_MAX_QUEUE_SIZE];
        unsigned long m_aTimestamps[MESSAGE_MAX_QUEUE_SIZE];
        unsigned int m_nBufferIdx{0};
    };

    class ClMessageQueue
    {
        public:
            ~ClMessageQueue(){DeInit();}
            bool Init(ClMQTTClientI* pMQTTClient, const String * pFeeds, unsigned int nFeeds, Stream * pDbgLog=nullptr);
            bool AddMessage(unsigned int nFeedIdx, const String& sMsg, unsigned long nTimestamp_tenth, bool bDisconnectWhenSendFinished=false);

            size_t m_nErrorCount{0};
            size_t m_nPublishCount{0};
            int m_nConnectionError{0};
        private:
            void DeInit();
            Stream* m_pDbgLog{nullptr};
            StBuffer * m_pBuffers{nullptr};
            unsigned int m_nBufferCount{0};
            ClMQTTClientI * m_pMQTTClient{nullptr};
            const unsigned int m_nMaxReconnections{5};
            bool m_bSendJson{true};

            bool AddMessageToBuffer(unsigned int nFeedIdx, const String& sMsg, unsigned long nTimestamp_tenth);
            bool Send(bool bDisconnectWhenSendFinished=false);
            bool SendIterative();
            bool SendJson();
    };
}