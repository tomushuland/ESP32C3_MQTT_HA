/*
For the ESP32-C3. Prepared to send the data of the sensors to Homeassistant by MQTT. 
When it start up the device, it will send a dyscovery messages with all the sensors specified at "mqtt_sensors".

 IMPORTAN!! complete the config.h with your configuration
*/


#include <Wire.h>
#include "AHT10.h"
#include "mqtt.h"
#include "config.h"

// Serial for debug
#define SERIAL_BAUD 115200

// Time between reads of sensors and messages to the MQTT broker
#define TIMEBETWEENMESSAGES 10000  // 60000 = 1 minutes; 10000 = 10 seconds
long lastTime = 0; // save the last time

// Initialization of the WiFi and the MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Sensor AHT10 by I2C
AHT10 myAHT10(AHT10_ADDRESS_0X38);

// To add new sensors, add it at the next list and its pointer to access de value directly
// Sensors to be published at MQTT. Add your necesary sensors by adding or modifying {"sensor", "units", "icon", default value}
mqtt_sensors sensors[] = {
  {"temp", "°C", "mdi:thermometer", 0.0},
  {"hum", "%", "mdi:water-percent", 0.0},
  {"dewPoint", "°C", "mdi:water-thermometer-outline", 0.0},
  {"heatIndex", "°C", "mdi:sun-thermometer", 0.0},
};
// Pointers to access de values of the sensors directly
float& temp = sensors[0].value;
float& hum = sensors[1].value;
float& dewPoint = sensors[2].value;
float& heatIndex = sensors[3].value;

// Sensors prepared to get data to change some sensors. For example to change a LED
mqtt_switchs switchs[] = {
  {"led", "mdi:led-outline", 0.0}
};
float& led = switchs[0].value;


// Read values of the different sensors
void read_sensors_data(){
  // get AHT10 data
  myAHT10.getAHT10Data(&temp,&hum,&heatIndex,&dewPoint);
  
  // Debug: Print sensor values dynamically
  Serial.print("Sensor readings:");
  for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); ++i) {
    Serial.print("  ");
    Serial.print(sensors[i].name);
    Serial.print(": ");
    Serial.print(sensors[i].value);
    Serial.print(sensors[i].unit);
  }
  Serial.println("");
}




///////////// ARDUINO SETUP /////////////////////////
void setup()
{
    // Initialize I2C for the BME sensor
    Wire.begin();

    delay(100);
    // Initialize serial for debug
    Serial.begin(SERIAL_BAUD);
    delay(100);
    Serial.println("\n\n***************************\n Starting sensorico!!!\n***************************");


    // AHT10 connection check
    if(!myAHT10.begin()){
       Serial.println("Could not find AHT10 sensor!");
    }else{
      Serial.println("AHT10 sensor initialized!");
    }

    // Initializate Wi-Fi & MQTT
    mqtt_reconnect(sensors, MQTT_DATA_SIZE(sensors), switchs, MQTT_DATA_SIZE(switchs));
}

///////////// ARDUINO LOOP /////////////////////////
void loop()
{
    long now = millis();
    if (now - lastTime > TIMEBETWEENMESSAGES || lastTime == 0 ) {
        lastTime = now;
        
        read_sensors_data();
        mqtt_publish_data(sensors, MQTT_DATA_SIZE(sensors), switchs, MQTT_DATA_SIZE(switchs));
    }
}