#if BUILD_ENV_NAME == 'micro'
#define MICRO
#else
#define MKRZERO
#endif

#include <Arduino.h>
#include "sim7600.h"
#include "sensor_bme680.h"
#include "sleep_time.h"
#include "relay.h"
#include "voltage.h"

#define MQTT_PUB_TEMPERATURE_FEED "traeholm/temperature"
#define MQTT_PUB_HUMIDITY_FEED "traeholm/humidity"
#define MQTT_PUB_PRESSURE_FEED "traeholm/pressure"
#define MQTT_PUB_STATUS_FEED "traeholm/status"
#define MQTT_SUB_FEED_TIMING "traeholm/timing"
#define MQTT_SUB_FEED_RELAY "traeholm/relay"
#define MQTT_PUB_VOLTAGE1_FEED "traeholm/voltage1"
#define MQTT_PUB_VOLTAGE2_FEED "traeholm/voltage2"
#define MQTT_PUB_VOLTAGE3_FEED "traeholm/voltage3"

#define DEBUG //uncomment for debugging

//config end
#ifdef DEBUG
#ifdef MICRO
#include <SoftwareSerial.h>
#endif
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

void CreateStatus(int nErrorCode, unsigned long nDelay, unsigned long nLoopCounter, bool bRelay, String &sJson){
	sJson = "{";
	sJson += "\"error\": " + String(nErrorCode);
	sJson += ", \"delay\": " + String(nDelay);
	sJson += ", \"loop\": " + String(nLoopCounter);
	sJson += ", \"relay\": " +String(bRelay ? 1 : 0);
	sJson += "}";
}


#define MESSAGE_QUEUE_SIZE 7

void setup() {
delay(5000); //security wait
#ifdef DEBUG
	Serial.begin(SIM7600_BAUD_RATE);
#endif
	
	PRINTFLN("Serial ok");
	//SoftwareSerial oSer(ARDUINO_RX, ARDUINO_TX);
#ifdef DEBUG
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, &Serial);
#else
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE);
#endif
	const String cpFeeds[MESSAGE_QUEUE_SIZE] = { MQTT_PUB_TEMPERATURE_FEED, 
								MQTT_PUB_HUMIDITY_FEED,
								MQTT_PUB_PRESSURE_FEED,
								MQTT_PUB_STATUS_FEED, 
								MQTT_PUB_VOLTAGE1_FEED,
								MQTT_PUB_VOLTAGE2_FEED,
								MQTT_PUB_VOLTAGE3_FEED,};
						
	g_pMsgQueue = new SIM7600MQTT::ClMessageQueue();
	if(!g_pMsgQueue->Init(g_pSim7600, cpFeeds, MESSAGE_QUEUE_SIZE, &Serial)){
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
	g_pRelay = new ClRelay();
	g_pRelay->Init();
	PRINTFLN("g_pRelay ok");

#ifdef DEBUG
	g_pVoltage = new ClVoltageMeasurement(&Serial);
#else
	g_pVoltage = new ClVoltageMeasurement();
#endif
	PRINTFLN("g_pVoltage ok");
}

unsigned long g_nLoopDelay{5000};
unsigned long g_nLoopCount{0};
unsigned long g_nCurrentTimeStamp_tenth{0};
unsigned long g_nMillis0{0};

void ISR(){
	digitalWrite(LED_BUILTIN, HIGH);
}

unsigned long tenth(){
	return g_nCurrentTimeStamp_tenth + (millis() -g_nMillis0)/100;
}

void sleep(bool bEnergySavingMode=true){
	if(bEnergySavingMode){
		g_rtc.setTime(0,0,0);
		g_rtc.setDate(24,05,2022);
		uint8_t nSleepSeconds = static_cast<uint8_t>(g_nLoopDelay/1000UL);
		uint8_t nSleepMinutes = nSleepSeconds/60;
		nSleepSeconds = nSleepSeconds%60;
		PRINTLN(String(F("Wait for ")) + String(nSleepMinutes) + String("min ") + String(nSleepSeconds) + F("s"));
		g_rtc.setAlarmTime(0, nSleepMinutes, nSleepSeconds);
		g_rtc.enableAlarm(g_rtc.MATCH_HHMMSS);
		g_rtc.attachInterrupt(ISR);
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
			g_nLoopDelay = max(3000UL, min(120000UL, nLoopDelay));
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
				g_pRelay->On();
			}
			else if(sRelayMsg == "OFF"){
				PRINTF("relay off!");
				g_pRelay->Off();
			}
		}
		else{
			PRINTF("failed to get relay state");
			nErrorCode=4;
		}

		g_pSim7600->disconnect();
	}

	g_pRelay->Check();	

	String sStatusJSON;
	CreateStatus(nErrorCode, g_nLoopDelay, g_nLoopCount, g_pRelay->Status(), sStatusJSON);
	Check_Queue(g_pMsgQueue->AddMessage(3, sStatusJSON, tenth(), true), bDummy);
	g_nLoopCount++;
	sleep(true);
}
