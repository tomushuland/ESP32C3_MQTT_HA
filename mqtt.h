#ifndef MQTT_H
#define MQTT_H

// #include <ESP8266WiFi.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_sleep.h"

// defined in pubSub MQTT_MAX_PACKET_SIZE 512
#define MQTT_MAX_PACKET_SIZE 512

const unsigned long WiFiconnectInterval = 30000; // 30 seconds



#define PUB_TOPIC 0
#define SUB_TOPIC 1

// Description of the device
extern const char* NAME;
extern const char* MANUFACTURER;
extern const char* MODEL;
extern const char* VERSION;


// Update these with values suitable for your WiFi network and MQTT.
extern const char* ssid ;
extern const char* pswd;
extern const char* mqtt_server;
extern const uint16_t mqtt_port;  
extern const char* mqtt_user;  
extern const char* mqtt_pass; 

extern const char* topic;    // rhis is the [root topic]
extern const char* pub;    // this is the subtopic publisher
extern const char* sub;     // this is the subtopic subscribed

extern PubSubClient client;


// estructura de la inicialización de los sensores
struct mqtt_sensors {
  const char* name;  // nombre del sensor
  const char* unit;  // Unidades del sensor
  const char* icon;  // Icono que usará
  float value;
};

// estructura de la inicialización de los sensores
struct mqtt_switchs {
  const char* name;  // nombre del sensor
  const char* icon;  // Icono que usará
  float value;
};

#define MQTT_DATA_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


void setup_wifi();

void mqtt_callback(char* topic, byte* payload, unsigned int length);

String mqtt_json_detecion(const char* sensor);
String mqtt_add_MAC_to_Sensor(const char* sensor);
String mqtt_composeTopic(bool pub_sub_option);

void mqtt_discovery_sensor(const char* sensor, const char* unit_of_measurement, const char* icon);
void mqtt_discovery_switch(const char* sensor, const char* icon);
void mqtt_reconnect(mqtt_sensors* sensors, size_t sensor_count, mqtt_switchs* switchs, size_t switchs_count);
bool mqtt_publish_data(mqtt_sensors* sensors, size_t sensor_count, mqtt_switchs* switchs, size_t switchs_count);

#endif // MQTT_H