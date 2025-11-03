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
#define DEBUG 0     // 1: debug enabled, 0: debug disabled
#define SERIAL_BAUD 115200

#define KEY_PIN 0 // GPIO of the ESP32 connected to KEY pin of the MH-CD42
#define SLEEP_DURATION_SECONDS 20 // should be less than the timeout of the MH-CD42!

// Time between reads of sensors and messages to the MQTT broker
#define TIMEBETWEENSAMPLES_SECONDS 300
RTC_DATA_ATTR uint16_t sleep_cycle_counter = 0;
const int CYCLES_PER_SAMPLE = TIMEBETWEENSAMPLES_SECONDS / SLEEP_DURATION_SECONDS; // How many cycles of SLEEP_DURATION_SECONDS needed to get the TIMEBETWEENSAMPLES_SECONDS

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

// Sensor AHT10 by I2C
AHT10 myAHT10(AHT10_ADDRESS_0X38);

// Initialization of the WiFi and the MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void debug_print(const String &msg) {
#if DEBUG
  Serial.println(msg);
#endif
}



// Read values of the different sensors
void read_sensors_data(){
  // get AHT10 data
  myAHT10.getAHT10Data(&temp,&hum,&heatIndex,&dewPoint);
#if DEBUG
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
#endif
}




///////////// ARDUINO SETUP /////////////////////////
void setup()
{
    // Config PIN conected to KEY of the MH-CD42
    pinMode(KEY_PIN, OUTPUT);
    digitalWrite(KEY_PIN, HIGH); // inactive

    // Initialize I2C for the BME sensor
    Wire.begin();
    delay(50);

#if DEBUG
    // Initialize serial for debug
    Serial.begin(SERIAL_BAUD);
    delay(100);
    Serial.println("\n\n***************************\n Starting sensorico!!!\n***************************");
#endif


  // Counter of cycles
  sleep_cycle_counter++;
  debug_print("Sleep cycle: " + String(sleep_cycle_counter) + " / " + String(CYCLES_PER_SAMPLE));

  // Check if it is needed to read the sensors and publish
  if (sleep_cycle_counter >= CYCLES_PER_SAMPLE) {
    debug_print("Time to work! Reading sensors and publishing.");

    sleep_cycle_counter = 0; // reset cycle

    // AHT10 connection check
    if(!myAHT10.begin()){
      debug_print("Could not find AHT10 sensor!");
    }
    else{
      debug_print("AHT10 sensor initialized!");
      read_sensors_data();
    }

    // Initializate Wi-Fi & MQTT
    mqtt_reconnect(sensors, MQTT_DATA_SIZE(sensors), switchs, MQTT_DATA_SIZE(switchs));
    // And Publish the datas
    if(mqtt_publish_data(sensors, MQTT_DATA_SIZE(sensors), switchs, MQTT_DATA_SIZE(switchs))){
      debug_print("Data published successfully.");
      delay(100); // to ensure transmision
    } 
    else {
      debug_print("Failed to publish data.");
    }
  }
  else{
    debug_print("Just a keep-alive cycle. Going back to sleep.");
  }

  // Pulse "keep-Alive" for the MH-CD42. Trigger the KEY button for (100-200ms)
  debug_print("Sending Keep-Alive pulse to MH-CD42...");
  digitalWrite(KEY_PIN, LOW);
  delay(150); // Mantenemos el pulso 150ms
  digitalWrite(KEY_PIN, HIGH);


  // Entering DEEP SLEEP mode
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000); // enable the timer. Convert to seconds to microseconds
  debug_print("Going to sleep for " + String(SLEEP_DURATION_SECONDS) + " seconds...");
  esp_deep_sleep_start();
}

///////////// ARDUINO LOOP /////////////////////////
void loop(){}