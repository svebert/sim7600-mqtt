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

#define NO_SERIAL //comment for debugging

#ifndef NO_SERIAL
#include <SoftwareSerial.h>
#endif

#ifndef NO_SERIAL
#define PRINT(X) Serial.print(X);
#define PRINTLN(X) Serial.println(X);
#else
#define PRINT(X)
#define PRINTLN(X)
#endif

void publish_measurement(String sFeed, float fValue){

	PRINT(sFeed)
	delay(250);
	if(pSim7600->publish(sFeed, String(fValue, 2)) != 0){
		PRINTLN("...failed")
	}
	PRINTLN("...ok")
}

void setup() {
#ifndef NO_SERIAL
	Serial.begin(SIM7600_BAUD_RATE);
	while (!Serial){
		delay(2);
	}
#endif
	
	//SoftwareSerial oSer(ARDUINO_RX, ARDUINO_TX);
#ifndef NO_SERIAL
	pSim7600 = new SIM7600MQTT::ClMQTTClient(SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, &Serial);
#else
	pSim7600 = new SIM7600MQTT::ClMQTTClient(SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE);
#endif
	pBME680 = new ClBME680Wrapper();
	if(!pBME680->init()){
		 PRINTLN("Could not find a valid BME680 sensor, check wiring!")
		 pBME680->deinit();
	}
}

int gnLoopDelay{5000};

void loop() 
{
	if(!pBME680->performReading()){
		PRINTLN("Failed reading BME680 sensor");
	}	

	if(!pSim7600->isConnected())
	{
		PRINT("connect ... ")
		if(pSim7600->connect(SIM7600_MQTT_HOST, SIM7600_MQTT_PORT, SIM7600_MQTT_HOST_USERNAME,
		 SIM7600_MQTT_HOST_KEY) != 0)
		{
			PRINTLN("failed")
			pSim7600->disconnect();
			delay(10000);
			return;
		}
		PRINTLN("ok")
	}

	publish_measurement(MQTT_PUB_TEMPERATURE_FEED, pBME680->temperature() + CALIB_TEMP);
	publish_measurement(MQTT_PUB_PRESSURE_FEED, pBME680->pressure());
	publish_measurement(MQTT_PUB_HUMIDITY_FEED, pBME680->humidity());
	// publish_measurement(MQTT_PUB_GASRESISTANCE_FEED, pBME680->gas_resistance());
	//publish_measurement(MQTT_PUB_VOLTAGE_FEED, 10.0);
	PRINT("subscribe (retained)...");
	String sSubMsg;
	if(pSim7600->get_subscribe(MQTT_SUB_FEED, sSubMsg) != 0)
	{
			PRINTLN("failed")
			delay(10000);
			return;
	}
	else{
		PRINTLN(String("Message: ") + sSubMsg)
		gnLoopDelay = atoi(sSubMsg.c_str());
	}
	gnLoopDelay = max(3000, min(120000, gnLoopDelay));

	PRINTLN(String("Wait for ") + String(gnLoopDelay) + "ms")
	delay(gnLoopDelay);
}
