#include "mqtt.h"

int status = WL_IDLE_STATUS;     // the starting Wifi radio's status


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pswd);
  unsigned long wifiConnectAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiConnectAttemptTime <= WiFiconnectInterval) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED){
      Serial.println("");
    Serial.print("WiFi connected.");
    Serial.print("   IP address: ");
    Serial.print(WiFi.localIP());

    // Update the MQTT buffer
    client.setBufferSize(MQTT_MAX_PACKET_SIZE);
    Serial.print("   MQTT Buffer size: ");
    Serial.println(client.getBufferSize()); 
  }
  else{
    Serial.println("\nError connecting to WiFi.");
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  // if ((char)payload[0] == '1') {
  //   digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //   // but actually the LED is on; this is because
  //   // it is acive low on the ESP-01)
  // } else {
  //   digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  // }

  String payload_send = "{\"led\":";
  payload_send += 0;
  payload_send += "}";
  Serial.print("Publish topic: ");
  Serial.println(mqtt_composeTopic(PUB_TOPIC));
  Serial.print("Publish message: ");
  Serial.println(payload_send);
  client.publish( (char*) mqtt_composeTopic(PUB_TOPIC).c_str() , (char*) payload_send.c_str(), true );
    
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    // if (i < 5)
    //   result += '_';
  }
  return result;
}


String mqtt_composeClientID() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId;
  clientId += macToStr(mac);
  return clientId;
}

// Compose the topic. Publish = 0, Subscribed = 1
// topic/MAC/pub  or  topic/MAC/sub
String mqtt_composeTopic(bool pub_sub_option){
  String pubTopic;

  pubTopic += topic ;
  pubTopic += "/";
  pubTopic += mqtt_composeClientID() ;

  if (!pub_sub_option){
    pubTopic += pub;
  }
  else{
    pubTopic += sub;
  }

  return pubTopic;
}

// MAC_sensor
String mqtt_json_detecion(const char* sensor){
  String adition;
  adition += "{{ value_json." ;
  adition += sensor;
  adition += "}}" ;

  return adition;
}

// MAC_sensor
String mqtt_add_MAC_to_Sensor(const char* sensor){
  String adition;
  adition += mqtt_composeClientID() ;
  adition += "_" ;
  adition += sensor;

  return adition;
}

// part of the manufacturer payload
String mqtt_discovery_device_payload(){
  String payload;
  payload += "\"dev\":{\"ids\":\"";      payload += NAME;payload += "_";payload += mqtt_composeClientID(); // Un identificador único del dispositivo
  payload += "\",\"name\":\"";           payload += NAME;  // El nombre del dispositivo.
  payload += "\",\"mf\": \"";            payload += MANUFACTURER;  // El fabricante del dispositivo.
  payload += "\",\"mdl\": \"";           payload += MODEL;  // El modelo del dispositivo.
  payload += "\",\"sn\": \"";            payload += "sn_";payload += mqtt_composeClientID(); // El número de serie del dispositivo.
  payload += "\",\"sw\": \"";            payload += VERSION;  // La versión de software del dispositivo.
  payload += "\"}";

  return payload;
}


// one discovery per sensor
void mqtt_discovery_sensor(const char* sensor, const char* unit_of_measurement, const char* icon){
  // Topic for discovery: <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  // Topic: homeassistant/sensor/nombresensor/config
  String payload = "{";
  payload += "\"name\":\"";             payload += sensor; // Especifica la clase del dispositivo, lo que ayuda a Home Assistant a presentar el sensor con un ícono adecuado y a interpretar correctamente sus datos. En este caso, la clase es "temperature" (temperatura)
  payload += "\",\"stat_t\":\"";         payload += mqtt_composeTopic(0);  // Define el topic de MQTT donde el dispositivo publica su estado. Home Assistant se suscribirá a este topic para recibir actualizaciones de estado del sensor.
  payload += "\",\"unit_of_meas\":\"";   payload += unit_of_measurement;  // Especifica la unidad de medida de los datos del sensor. En este caso, la temperatura se mide en grados Celsius (°C).
  payload += "\",\"ic\":\"";              payload += icon;
  payload += "\",\"val_tpl\":\"";        payload += mqtt_json_detecion(sensor); // Utiliza una plantilla de Jinja2 para extraer el valor del estado del mensaje JSON recibido. Aquí, extrae el valor de la temperatura del campo temperature del JSON.
  payload += "\",\"uniq_id\":\"";        payload += mqtt_add_MAC_to_Sensor(sensor); // Proporciona un identificador único para el sensor, lo que permite a Home Assistant distinguir este dispositivo de otros.
  payload += "\",\"obj_id\":\"";         payload += NAME;payload += "_";payload += mqtt_add_MAC_to_Sensor(sensor); // Proporciona un identificador único para el sensor, lo que permite a Home Assistant distinguir este dispositivo de otros.
  payload += "\",";
  // device: Proporciona información adicional sobre el dispositivo al que pertenece el sensor. Dentro de este objeto
  payload += mqtt_discovery_device_payload();
  payload += "}";

  Serial.print("Publish discovery: ");
  String dyscover_topic ="homeassistant/sensor/";dyscover_topic += mqtt_add_MAC_to_Sensor(sensor); dyscover_topic += "/config";
  Serial.println(dyscover_topic);
  Serial.print("Publish message: ");
  Serial.println(payload);
  
  
  if(!client.publish((char*) dyscover_topic.c_str() , (char*) payload.c_str(), true )){
    Serial.print("failed, rc=");
    Serial.print(client.state());    
    Serial.println(WiFi.status());
    Serial.println(" ERROR publishing the mesage");

  }
  else{
    // Serial.println("Published");
  }
}


// one discovery per sensor
void mqtt_discovery_switch(const char* sensor, const char* icon){
  // Topic for discovery: <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  // Topic: homeassistant/sensor/nombresensor/config
  String payload = "{";
  payload += "\"name\":\"";              payload += sensor; // Especifica la clase del dispositivo, lo que ayuda a Home Assistant a presentar el sensor con un ícono adecuado y a interpretar correctamente sus datos. En este caso, la clase es "temperature" (temperatura)
  payload += "\",\"cmd_t\":\"";          payload += mqtt_composeTopic(1);  // Define el topic de MQTT donde el dispositivo subcribe para recibir comandos.
  payload += "\",\"stat_t\":\"";         payload += mqtt_composeTopic(0);  // Define el topic de MQTT donde el dispositivo publica su estado. Home Assistant se suscribirá a este topic para recibir actualizaciones de estado del sensor.
  payload += "\",\"ic\":\"";             payload += icon;
  payload += "\",\"val_tpl\":\"";        payload += mqtt_json_detecion(sensor); // Utiliza una plantilla de Jinja2 para extraer el valor del estado del mensaje JSON recibido. Aquí, extrae el valor de la temperatura del campo temperature del JSON.
  payload += "\",\"uniq_id\":\"";        payload += mqtt_add_MAC_to_Sensor(sensor); // Proporciona un identificador único para el sensor, lo que permite a Home Assistant distinguir este dispositivo de otros.
  payload += "\",\"obj_id\":\"";         payload += NAME;payload += "_";payload += mqtt_add_MAC_to_Sensor(sensor); // Proporciona un identificador único para el sensor, lo que permite a Home Assistant distinguir este dispositivo de otros.
  payload += "\",\"pl_on\":\"";          payload += "1";
  payload += "\",\"pl_off\":\"";         payload += "0";
  payload += "\",\"ret\":\"";            payload += "true";
  payload += "\",";
  // device: Proporciona información adicional sobre el dispositivo al que pertenece el sensor. Dentro de este objeto
  payload += mqtt_discovery_device_payload();
  payload += "}";

  Serial.print("Publish discovery: ");
  String dyscover_topic ="homeassistant/switch/"; dyscover_topic += mqtt_add_MAC_to_Sensor(sensor); dyscover_topic += "/config";
  Serial.println(dyscover_topic);
  Serial.print("Publish message: ");
  Serial.println(payload);
  client.publish((char*) dyscover_topic.c_str() , (char*) payload.c_str(), true );
}


void mqtt_reconnect(mqtt_sensors* sensors, size_t sensor_count, mqtt_switchs* switchs, size_t switchs_count) {
  // Check if there is connection to the WiFi and try to connect
  if (WiFi.status() != WL_CONNECTED){
    setup_wifi();
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);


  Serial.print("Attempting MQTT connection...");
 
  // Attempt to connect
  if (client.connect((char*) mqtt_composeClientID().c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("connected");
    client.subscribe(mqtt_composeTopic(SUB_TOPIC).c_str() );
    Serial.print("subscribed to : ");
    Serial.println(mqtt_composeTopic(SUB_TOPIC));

    // Configure the auto discovery of the sensors through the MQTT
    for (size_t i = 0; i < sensor_count; ++i) {
      mqtt_discovery_sensor(sensors[i].name, sensors[i].unit, sensors[i].icon);
    }

    // Configure the auto discovery of the switches through the MQTT
    for (size_t i = 0; i < switchs_count; ++i) {
      mqtt_discovery_switch(switchs[i].name, switchs[i].icon);
    }


  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.print(" wifi=");
    Serial.println(WiFi.status());
  }
}

// Publish all the data to the MQTT server dynamically
bool mqtt_publish_data(mqtt_sensors* sensors, size_t sensor_count, mqtt_switchs* switchs, size_t switchs_count) {
  // Confirm still connected to MQTT server
  if (!client.connected()) {
      mqtt_reconnect(sensors, sensor_count, switchs, switchs_count);
  }

  // If connected try to send the data
  if (client.connected()){
    // Process messages and maitain the MQTT connection
    client.loop();

    // Construct JSON payload dynamically
    String payload = "{";
    for (size_t i = 0; i < sensor_count; ++i) {
        payload += "\"";
        payload += sensors[i].name;
        payload += "\":";
        payload += sensors[i].value;
        if (i < sensor_count - 1) {
            payload += ",";
        }
    }
    payload += "}";

    Serial.print("Publish topic: ");
    Serial.println(mqtt_composeTopic(PUB_TOPIC));
    Serial.print("Publish message: ");
    Serial.println(payload);

    // Publish the payload to the MQTT broker
    client.publish((char*)mqtt_composeTopic(PUB_TOPIC).c_str(), (char*)payload.c_str(), true);
    return true;
  }
  return false;
}
