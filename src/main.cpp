#include <Arduino.h>
#define DEBUG //uncomment for debugging
#define MQTT_DUMMY //uncomment to use mqtt dummy class

#include "sim7600.h"
#include "sensor_bme680.h"
#include "sleep_time.h"
#include "relay.h"
#include "voltage.h"
#include "reset.h"

#define MQTT_PUB_BASE "traeholm"
#define MQTT_PUB_TEMPERATURE_FEED MQTT_PUB_BASE "/temperature"
#define MQTT_PUB_HUMIDITY_FEED MQTT_PUB_BASE "/humidity"
#define MQTT_PUB_PRESSURE_FEED MQTT_PUB_BASE "/pressure"
#define MQTT_PUB_STATUS_FEED MQTT_PUB_BASE "/status"
#define MQTT_SUB_FEED_TIMING MQTT_PUB_BASE "/timing"
#define MQTT_SUB_FEED_RELAY MQTT_PUB_BASE "/relay"
#define MQTT_PUB_VOLTAGE1_FEED MQTT_PUB_BASE "/voltage1"
#define MQTT_PUB_VOLTAGE2_FEED MQTT_PUB_BASE "/voltage2"
#define MQTT_PUB_VOLTAGE3_FEED MQTT_PUB_BASE "/voltage3"

constexpr unsigned long g_nResetCount = ((400/MESSAGE_MAX_QUEUE_SIZE)*MESSAGE_MAX_QUEUE_SIZE);
//config end
#ifdef DEBUG
#define PRINTF(X) Serial.print(F(X));
#define PRINTFLN(X) Serial.println(F(X));
#define PRINT(X) Serial.print(X);
#define PRINTLN(X) Serial.println(X);
#else
#define PRINTFLN(X)
#define PRINTF(X)
#define PRINT(X)
#define PRINTLN(X)
#endif

void Check_Queue(bool /*bReturnQueue*/, bool& bIsConnected){
	bIsConnected = false;
	if(g_pMsgQueue->m_nConnectionError == 1){
		PRINTFLN("!!failed to connect");
	}
	else if(g_pMsgQueue->m_nConnectionError == 2){
		PRINTFLN("!!failed to disconnect");
	}
	else{
		if(g_pMsgQueue->m_nErrorCount > 0){

			PRINTLN(String(F("!!Failed to send all messages: ")) + String(g_pMsgQueue->m_nErrorCount) + String(F("/")) + String(g_pMsgQueue->m_nPublishCount));
			if(g_pMsgQueue->m_nPublishCount > 0){
				bIsConnected = true;
			}
		}
		else if(g_pMsgQueue->m_nPublishCount == 0){
			//PRINTFLN("!!added to buffer");
		}
		else{
			PRINTFLN("!!send");
			bIsConnected = true;
		}
	}	
}

void CreateStatus(int nErrorCode, unsigned long nDelay, unsigned long nLoopCounter, bool bPowerRelay, String &sJson){
	sJson = "{";
	sJson += "\"error\": " + String(nErrorCode);
	sJson += ", \"delay\": " + String(nDelay);
	sJson += ", \"loop\": " + String(nLoopCounter);
	sJson += ", \"powerrelay\": " +String(bPowerRelay ? 1 : 0);
	sJson += "}";
}


#define MESSAGE_FEED_COUNT 7

void setup() {
delay(5000); //security wait
#ifdef DEBUG
	Serial.begin(SIM7600_BAUD_RATE);
#endif
	
	PRINTFLN("Serial ok");

#ifdef DEBUG
	#ifdef MQTT_DUMMY
	g_pSim7600 = new SIM7600MQTT::ClMQTTClientDummy();
	#else
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, &Serial, HOME_AUTH_SIM7600_APN);
	#endif
#else
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, nullptr, HOME_AUTH_SIM7600_APN);
#endif
	const String cpFeeds[MESSAGE_FEED_COUNT] = { MQTT_PUB_TEMPERATURE_FEED, 
								MQTT_PUB_HUMIDITY_FEED,
								MQTT_PUB_PRESSURE_FEED,
								MQTT_PUB_STATUS_FEED, 
								MQTT_PUB_VOLTAGE1_FEED,
								MQTT_PUB_VOLTAGE2_FEED,
								MQTT_PUB_VOLTAGE3_FEED,};
						
	g_pMsgQueue = new SIM7600MQTT::ClMessageQueue();
	if(!g_pMsgQueue->Init(g_pSim7600, cpFeeds, MESSAGE_FEED_COUNT, &Serial)){
		 PRINTFLN("Could not init MsgQueue");			
	}
	PRINTFLN("g_pMsgQueue ok");
	g_pBME680 = new ClBME680Wrapper();
	if(!g_pBME680->init()){
		 PRINTFLN("Could not find a valid BME680 sensor, check wiring!")
		 g_pBME680->deinit();
	}
	PRINTFLN("g_pBME680 ok");

	g_rtc.begin();
	PRINTFLN("g_rtc ok");
	g_pPowerRelay = new ClRelay();
	g_pPowerRelay->Init();
	PRINTFLN("g_pPowerRelay ok");

#ifdef DEBUG
	g_pVoltage = new ClVoltageMeasurement(&Serial);
#else
	g_pVoltage = new ClVoltageMeasurement();
#endif
	PRINTFLN("g_pVoltage ok");

	g_pReset = new ClReset();
	g_pReset->Init();
	PRINTFLN("g_pReset ok");
}

unsigned long g_nLoopDelay{5000};
unsigned long g_nLoopCount{0};
unsigned long g_nCurrentTimeStamp_tenth{0};
unsigned long g_nMillis0{0};

void ISR(){
	digitalWrite(LED_BUILTIN, HIGH);
}

//internal current time unit: tenth of a second
unsigned long tenth(){
	return g_nCurrentTimeStamp_tenth + (millis() -g_nMillis0)/100;
}

void sleep(bool bEnergySavingMode=true){
	if(bEnergySavingMode){
		g_rtc.setTime(0, 0, 0);
		g_rtc.setDate(24,5,22);
		unsigned long nSleepSeconds = g_nLoopDelay/1000UL;
		uint8_t nSleepMinutes = nSleepSeconds/60;
		uint8_t nSleepSeconds_uint8 = static_cast<uint8_t>(nSleepSeconds%60);
		PRINTLN(String(F("Wait for ")) + String(nSleepMinutes) + String("min ") + String(nSleepSeconds_uint8) + F("s"));
		g_rtc.setAlarmTime(0, nSleepMinutes, nSleepSeconds_uint8);
		#ifdef MKRZERO
		g_rtc.enableAlarm(g_rtc.MATCH_HHMMSS);
		g_rtc.attachInterrupt(ISR);
		#endif
		digitalWrite(LED_BUILTIN, LOW);
		g_nCurrentTimeStamp_tenth =  tenth() + g_nLoopDelay/100;
		g_rtc.standbyMode();
	}
	else{
		g_nCurrentTimeStamp_tenth =  tenth() + g_nLoopDelay/100;
		delay(g_nLoopDelay);
	}
}

void loop() 
{
	g_nMillis0 = millis();
	int nErrorCode = 0;
	PRINTLN(String("measure(") + String(g_nLoopCount) + String(")..."));
	if(!g_pBME680->performReading()){
		PRINTFLN("Failed reading BME680 sensor");nErrorCode=1;
		if(!g_pBME680->init()){
		 	PRINTFLN("Could not find a valid BME680 sensor, check wiring!");nErrorCode=2;
		 	g_pBME680->deinit();
		}
	}

	PRINTFLN("add messages...");
	bool bIsConnected, bDummy;
	unsigned long nTSSensor = tenth();
	Check_Queue(g_pMsgQueue->AddMessage(0, String(g_pBME680->temperature(), 2), nTSSensor), bIsConnected);
	Check_Queue(g_pMsgQueue->AddMessage(1, String(g_pBME680->humidity(), 1), nTSSensor), bDummy);
	Check_Queue(g_pMsgQueue->AddMessage(2, String(g_pBME680->pressure(), 2), nTSSensor), bDummy);

	Check_Queue(g_pMsgQueue->AddMessage(4, String(g_pVoltage->MeasureVoltage(0), 1), tenth()), bDummy);
	Check_Queue(g_pMsgQueue->AddMessage(5, String(g_pVoltage->MeasureVoltage(1), 1), tenth()), bDummy);
	Check_Queue(g_pMsgQueue->AddMessage(6, String(g_pVoltage->MeasureVoltage(2), 1), tenth()), bDummy);

	if(bIsConnected){
		PRINTF("get messages...");
		unsigned long nLoopDelay;
		PRINTF("timing:");
		if(g_pSim7600->GetMessage(MQTT_SUB_FEED_TIMING, nLoopDelay))
		{
			PRINTLN(String("delay=") + String(nLoopDelay) );
			g_nLoopDelay = max(3000UL, min(600000UL, nLoopDelay));
		}
		else{
			PRINTF("failed to get dealy");
			nErrorCode=3;
		}
		String sRelayMsg;
		PRINTF("relay:");
		if(g_pSim7600->GetMessage(MQTT_SUB_FEED_RELAY, sRelayMsg))
		{
			PRINTLN(sRelayMsg);
			if(sRelayMsg == "ON"){
				PRINTF("relay on!");
				g_pPowerRelay->On();
			}
			else if(sRelayMsg == "OFF"){
				PRINTF("relay off!");
				g_pPowerRelay->Off();
			}
		}
		else{
			PRINTF("failed to get relay state");
			nErrorCode=4;
		}

		g_pSim7600->disconnect();
	}

	g_pPowerRelay->Check();	

	String sStatusJSON;
	CreateStatus(nErrorCode, g_nLoopDelay, g_nLoopCount, g_pPowerRelay->Status(), sStatusJSON);
	Check_Queue(g_pMsgQueue->AddMessage(3, sStatusJSON, tenth(), true), bDummy);
	g_nLoopCount++;

	if( g_nLoopCount >= g_nResetCount){
		PRINTF("RESET!");
		g_pSim7600->reset();
		g_pReset->HardReset();
	}
	sleep(false);
}
