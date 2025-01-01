#ifndef CONFIG_H
#define CONFIG_H

// WiFi connection
const char* ssid = "your WiFi SSID";
const char* pswd = "password of your WiFi";

// Homeassitant configuration
const char* mqtt_server = "192.168.1.5";    // IP of the homeassistan
const uint16_t mqtt_port = 1883;            // port of the homeassistan. Usually 1883
const char* mqtt_user = "mqtt_user";        // Homeassistan MQTT user account for the device
const char* mqtt_pass = "mqtt_pass";        // Homeassistan MQTT password account for the device

// Homeassistant topics (not needed to be changed) --> topic/pub  or  topic/sub
const char* topic = "home/sensors";    // this is the [root topic]
const char* pub = "/out";    // this is the subtopic publisher
const char* sub = "/in";     // this is the subtopic subscribed

// Description of the device
const char* NAME = "MiniMeteo";
const char* MANUFACTURER = "ToMushuLand";
const char* MODEL = "ESP32-C3";
const char* VERSION = "0.1";

#endif