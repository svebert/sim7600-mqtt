#pragma once

#include "Adafruit_Auth.h"
#include <Sim7600-mqtt/message_queue.h>

#define SIM7600_BAUD_RATE 19200
#define SIM7600_ARDUINO_RX 8
#define SIM7600_ARDUINO_TX 9
#define SIM7600_MQTT_HOST "io.adafruit.com"
#define SIM7600_MQTT_PORT "1883"
#define SIM7600_MQTT_HOST_USERNAME ADA_AUTH_SIM7600_MQTT_HOST_USERNAME
#define SIM7600_MQTT_HOST_KEY ADA_AUTH_SIM7600_MQTT_HOST_KEY

SIM7600MQTT::ClMQTTClient * g_pSim7600;
SIM7600MQTT::ClMessageQueue * g_pMsgQueue;

const String g_sConnectionString("AT+CMQTTCONNECT=0,\"tcp://" SIM7600_MQTT_HOST ":" SIM7600_MQTT_PORT "\",90,1,\"" SIM7600_MQTT_HOST_USERNAME "\",\"" SIM7600_MQTT_HOST_KEY "\"");

