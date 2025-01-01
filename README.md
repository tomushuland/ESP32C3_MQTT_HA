# ESP32C3_MQTT_HA
ESP32-C3 Sensors connection to Homeassistant by MQTT with autodiscovery


For the ESP32-C3. 
Prepared to send the data of the sensors to Homeassistant by MQTT. Prepared with the AHT10 sensor
When it start up the device, it will send a dyscovery messages with all the sensors specified at "mqtt_sensors".

IMPORTAN!! complete the config.h with your configuration