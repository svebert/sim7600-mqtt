#pragma once

#include "mqtt.h"
#include <SoftwareSerial.h>

#define MESSAGE_QUEUE_SIZE 5
#define MESSAGE_QUEUE_FEED_COUNT 2
#define MESSAGE_QUEUE_MSG_LEN 8
#define MESSAGE_QUEUE_FEED_LEN 25
namespace SIM7600MQTT
{
    struct StMsgBuffer
    {
        char m_szMsg[MESSAGE_QUEUE_MSG_LEN];
    };

    struct StBuffer
    {
        char m_sFeed[MESSAGE_QUEUE_FEED_LEN];
        StMsgBuffer m_stElement[MESSAGE_QUEUE_SIZE];
        unsigned long m_aTimestamps[MESSAGE_QUEUE_SIZE];
    };

    class ClMessageQueue
    {
        public:
            bool Init(ClMQTTClient* pMQTTClient, const String * pFeeds, Stream * pDbgLog=nullptr);
            bool AddMessage(int nFeedIdx, const String& sMsg);

            size_t m_nErrorCount{0};
            size_t m_nPublishCount{0};
            int m_nConnectionError{0};
        private:
            Stream* m_pDbgLog{nullptr};
            StBuffer m_aBuffers[MESSAGE_QUEUE_FEED_COUNT];
            int GetFreeBuffer(int nFeedIdx);
            unsigned long m_nNow{0};
            ClMQTTClient * m_pMQTTClient{nullptr};
            const int m_nMaxReconnections{5};

            bool AddMessageToBuffer(int nFeedIdx, int nFreeBufferIdx, const String& sMsg);
            bool Send();
    };
}