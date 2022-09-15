#include <Arduino.h>
#include <SoftwareSerial.h>
#include "sim7600.h"
#include "sensor_bme680.h"

//#define MQTT_PUB_VOLTAGE_FEED "svebert/f/voltage"
#define MQTT_PUB_TEMPERATURE_FEED "svebert/f/temperature"
#define MQTT_PUB_PRESSURE_FEED "svebert/f/pressure"
#define MQTT_PUB_HUMIDITY_FEED "svebert/f/humidity"
#define MQTT_PUB_GASRESISTANCE_FEED "svebert/f/gas_resistance"
#define MQTT_SUB_FEED "svebert/f/welcome-feed"


void publish_measurement(String sFeed, float fValue){
	Serial.print(sFeed);
	if(pSim7600->publish(sFeed, String(fValue, 4)) != 0){
			Serial.println("...failed");
	}
	Serial.println("...ok");
}

void setup() {
	Serial.begin(SIM7600_BAUD_RATE);
	while (!Serial){
		delay(2);
	}
	
	//SoftwareSerial oSer(ARDUINO_RX, ARDUINO_TX);
	pSim7600 = new SIM7600MQTT::ClMQTTClient(SIM7600_ARDUINO_TX, SIM7600_ARDUINO_RX, SIM7600_BAUD_RATE, &Serial);
	pBME680 = new ClBME680Wrapper();
	if(!pBME680->init()){
		 Serial.println("Could not find a valid BME680 sensor, check wiring!");
		 pBME680->deinit();
	}
}

void loop() 
{
	if(!pBME680->performReading()){
		Serial.println("Failed reading BME680 sensor");
	}	

	if(!pSim7600->isConnected())
	{
		Serial.print("connect ... ");
		if(pSim7600->connect(SIM7600_MQTT_HOST, SIM7600_MQTT_PORT, SIM7600_MQTT_HOST_USERNAME,
		 SIM7600_MQTT_HOST_KEY) != 0)
		{
			Serial.println("failed");
			pSim7600->disconnect();
			delay(10000);
			return;
		}
		Serial.println("ok");
	}

	publish_measurement(MQTT_PUB_TEMPERATURE_FEED, pBME680->temperature());
	publish_measurement(MQTT_PUB_PRESSURE_FEED, pBME680->pressure());
	publish_measurement(MQTT_PUB_HUMIDITY_FEED, pBME680->humidity());
	publish_measurement(MQTT_PUB_GASRESISTANCE_FEED, pBME680->gas_resistance());
	//publish_measurement(MQTT_PUB_VOLTAGE_FEED, 10.0);
	Serial.print("subscribe (retained)...");
	String sSubMsg;
	if(pSim7600->get_subscribe(MQTT_SUB_FEED, sSubMsg) != 0)
	{
			Serial.println("failed");
			delay(10000);
			return;
	}
	else{
		Serial.println(String("Message: ") + sSubMsg);
	}

	delay(5000);
}
