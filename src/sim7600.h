#pragma once

#include <Sim7600-mqtt/mqtt.h>
#include "Adafruit_Auth.h"

#define SIM7600_BAUD_RATE 19200
#define SIM7600_ARDUINO_RX 8
#define SIM7600_ARDUINO_TX 9
#define SIM7600_MQTT_HOST "io.adafruit.com"
#define SIM7600_MQTT_PORT 1883
#define SIM7600_MQTT_HOST_USERNAME ADA_AUTH_SIM7600_MQTT_HOST_USERNAME
#define SIM7600_MQTT_HOST_KEY ADA_AUTH_SIM7600_MQTT_HOST_KEY

SIM7600MQTT::ClMQTTClient * pSim7600;
