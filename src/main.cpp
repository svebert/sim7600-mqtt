#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Sim7600-mqtt/mqtt.h>

#define BAUD_RATE 19200
#define ARDUINO_RX 8
#define ARDUINO_TX 9
#define MQTT_HOST "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_HOST_USERNAME "svebert"
#define MQTT_HOST_KEY "aio_YJVd18AnUslL9ngCwZVzdWCOVBaP"

#define MQTT_CLIENT_ID "sven-test-mqtt-id1"
#define MQTT_PUB_FEED "svebert/f/voltage"
#define MQTT_SUB_FEED "svebert/f/welcome-feed"


SIM7600MQTT::ClMQTTClient * pSim7600;

void setup() {
	Serial.begin(BAUD_RATE);
	//SoftwareSerial oSer(ARDUINO_RX, ARDUINO_TX);
	pSim7600 = new SIM7600MQTT::ClMQTTClient(ARDUINO_TX, ARDUINO_RX, BAUD_RATE, &Serial);
}

void loop() 
{
	// delay(10000);
	// return;
	if(!pSim7600->isConnected())
	{
		Serial.print("connect ... ");
		if(pSim7600->connect(MQTT_HOST, MQTT_PORT, MQTT_HOST_USERNAME, MQTT_HOST_KEY, MQTT_CLIENT_ID) != 0)
		{
			Serial.println("failed");
			pSim7600->disconnect();
			delay(10000);
			return;
		}
		Serial.println("ok");

		// Serial.print("subscribe ... ");
		// if(!pSim7600->subscribe("MQTT_SUB_FEED"))
		// {
		// 	Serial.println("failed");
		// }
		// Serial.println("ok");
	}

	Serial.print("publish ... ");
	if(pSim7600->publish(MQTT_PUB_FEED, "7.5") != 0)
	{
			Serial.println("failed");
			// pSim7600->disconnect();
			delay(10000);
			return;
	}
	Serial.println("ok");
	delay(5000);
}