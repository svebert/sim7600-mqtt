#include <Arduino.h>
#include "sim7600.h"
#include "sensor_bme680.h"

//#define MQTT_PUB_VOLTAGE_FEED "svebert/f/voltage"
#define CALIB_TEMP -1.5
#define MQTT_PUB_TEMPERATURE_FEED "svebert/f/temperature"
#define MQTT_PUB_PRESSURE_FEED "svebert/f/pressure"
#define MQTT_PUB_HUMIDITY_FEED "svebert/f/humidity"
// #define MQTT_PUB_GASRESISTANCE_FEED "svebert/f/gas-resistance"
#define MQTT_SUB_FEED "svebert/f/welcome-feed"

//#define NO_SERIAL //comment for debugging

#ifndef NO_SERIAL
#include <SoftwareSerial.h>
#endif

#ifndef NO_SERIAL
#define PRINTF(X) Serial.print(F(X));
#define PRINTFLN(X) Serial.println(F(X));
#define PRINT(X) Serial.print(X);
#define PRINTLN(X) Serial.println(X);
#else
#define PRINT(X)
#define PRINTLN(X)
#endif

void Check_Queue(bool bReturnQueue){
	if(!bReturnQueue)
	{
		if(g_pMsgQueue->m_nConnectionError == 1){
			PRINTFLN("!!failed to connect");
		}
		else if(g_pMsgQueue->m_nConnectionError == 2){
			PRINTFLN("!!failed to disconnect");
		}
		else{
			if(g_pMsgQueue->m_nErrorCount > 0){

				PRINTLN(String(F("!!Failed to send all messages: ")) + String(g_pMsgQueue->m_nErrorCount) + String(F("/")) + String(g_pMsgQueue->m_nPublishCount));
			}
			else if(g_pMsgQueue->m_nPublishCount == 0){
				PRINTFLN("!!added to buffer");
			}
			else{
				PRINTFLN("!!send");
			}
		}
	}
}


void setup() {
#ifndef NO_SERIAL
	Serial.begin(SIM7600_BAUD_RATE);
	while (!Serial){
		delay(2);
	}
#endif
	PRINTFLN("Serial ok");
	//SoftwareSerial oSer(ARDUINO_RX, ARDUINO_TX);
#ifndef NO_SERIAL
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, &Serial);
#else
	g_pSim7600 = new SIM7600MQTT::ClMQTTClient(g_sConnectionString, SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE);
#endif
	const String cpFeeds[2] = { MQTT_PUB_TEMPERATURE_FEED, 
								MQTT_PUB_HUMIDITY_FEED};
	g_pMsgQueue = new SIM7600MQTT::ClMessageQueue();
	if(!g_pMsgQueue->Init(g_pSim7600, cpFeeds, &Serial)){
		 PRINTFLN("Could not init MsgQueue");		
	}
	PRINTFLN("g_pMsgQueue ok");
	g_pBME680 = new ClBME680Wrapper();
	if(!g_pBME680->init()){
		 PRINTFLN("Could not find a valid BME680 sensor, check wiring!")
		 g_pBME680->deinit();
	}
	PRINTFLN("g_pBME680 ok");
}

unsigned long gnLoopDelay{5000};

unsigned long nLoopCount{0};

void loop() 
{
	PRINTLN(String("measure(") + String(nLoopCount) + String(")..."));
	if(!g_pBME680->performReading()){
		PRINTFLN("Failed reading BME680 sensor");
		if(!g_pBME680->init()){
		 	PRINTFLN("Could not find a valid BME680 sensor, check wiring!")
		 	g_pBME680->deinit();
		}
	}

	PRINTFLN("add messages...");
	Check_Queue(g_pMsgQueue->AddMessage(0, String(g_pBME680->temperature() + CALIB_TEMP)));
	//Check_Queue(g_pMsgQueue->AddMessage(1, String(g_pBME680->pressure())));
	Check_Queue(g_pMsgQueue->AddMessage(1, String(g_pBME680->humidity())));

	// Check_Queue(g_pMsgQueue->AddMessage(0, String(22.0)));
	// Check_Queue(g_pMsgQueue->AddMessage(1, String(1001.1)));
	// Check_Queue(g_pMsgQueue->AddMessage(2, String(57.5)));
	// publish_measurement(MQTT_PUB_GASRESISTANCE_FEED, pBME680->gas_resistance());
	//publish_measurement(MQTT_PUB_VOLTAGE_FEED, 10.0);
	if(nLoopCount % MESSAGE_QUEUE_SIZE == 0 && nLoopCount != 0)
	{
		PRINTF("subscribe (retained)...");
		if(!g_pSim7600->isConnected()){
			g_pSim7600->connect();
		}
		String sSubMsg;
		if(g_pSim7600->get_subscribe(MQTT_SUB_FEED, sSubMsg) != 0)
		{
				PRINTFLN("failed")
				delay(10000);
				return;
		}
		else{
			PRINTLN(String(F("Message: ")) + sSubMsg)
			gnLoopDelay = atoi(sSubMsg.c_str());
		}
		delay(250);
		g_pSim7600->disconnect();
		gnLoopDelay = max(3000UL, min(120000UL, gnLoopDelay));
	}
	PRINTLN(String(F("Wait for ")) + String(gnLoopDelay) + F("ms"))

	nLoopCount++;
	delay(gnLoopDelay);
}
