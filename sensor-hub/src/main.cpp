#include <Arduino.h>

// TODO:
/*
  Build IoT loop
  - Read and transmit data every 60 seconds (variable)
  - { "sensors": {  } }

  Build function for each sensor we are going to have
  - ultrasonic sensor first
*/

/*
  AWS IoT WiFi

  This sketch securely connects to an AWS IoT using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a public
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

  The circuit:
  - Arduino MKR WiFi 1010 or MKR1000

  The following tutorial on Arduino Project Hub can be used
  to setup your AWS account and the MKR board:

  https://create.arduino.cc/projecthub/132016/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365

  This example code is in the public domain.
*/

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include <Ultrasonic.h>
#include <ArduinoJson.h>
#include <max6675.h>

#include "arduino_secrets.h"

// Declare functions that other IDE does not requrie
void onMessageReceived(int messageSize);
void publishMessage(char *message);
void onJsonMessageReceived(int messageSize);
void processCommand(char* command);
void publishSensorData();
void publishHeraldMessage();
void connectMQTT();
void connectWiFi();
void printMacAddress(byte mac[]);
void printWifiStatus();
unsigned long getTime();

// MAX6675 Thermocouple settings and pin locations
int ktcSO = 8;
int ktcCS = 9;
int ktcCLK = 10;

MAX6675 thermocouple(ktcCLK, ktcCS, ktcSO);



/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

// Configuration parameters
const int message_interval = 5*60*1000; // time between sensor messages miliseconds
const char command_topic[] = "commands/gardenhub/maggie";
const char sensordata_topic[] = "sensordata/gardenhub/maggie";

// Some global state
WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

Ultrasonic ultrasonic(1);
unsigned long lastMillis = 0;

char command[128];

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onJsonMessageReceived);
}

void loop() {


  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    printMacAddress(mac);
    printWifiStatus();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
    publishHeraldMessage();
    //publishMessage("I am Maggie and I am allive");
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every interval
  if (millis() - lastMillis > message_interval) {
    lastMillis = millis();
    // TODO: Get sensor data here
    publishSensorData();
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(command_topic);
}

void publishMessage(char *message) {
  Serial.println("Publishing message");
  Serial.println(message);
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(sensordata_topic);
  mqttClient.print(message);
  mqttClient.print(millis());
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}

void onJsonMessageReceived(int messageSize) {
  StaticJsonDocument<256> mqtt_json_message;
  DeserializationError error = deserializeJson(mqtt_json_message, mqttClient);

  if (error) {
     Serial.print(F("deserializeJson() failed: "));
     Serial.println(error.c_str());
     return;
   }

   const char* mqtt_command = mqtt_json_message["command"];
   // Print values.
   Serial.println(mqtt_command);

   char received_conmman[256];
   strcpy(received_conmman, mqtt_command);
   processCommand(received_conmman);

}


// Implement the command handler -
void processCommand(char* command) {
  Serial.print("Processing a command ");
  Serial.println(command);
  // Eventually put a command processor switch statement here

  if (strcmp("post",command) == 0) {
    publishSensorData();
    return;
  }

  if (strcmp("herald",command) == 0){
    publishHeraldMessage();
    return;
  }

  // If we get here, we don't understand the command
  Serial.println("Unknown command ");
  publishMessage("{'message': 'error', 'error_message': 'Unkown command'}");
}

// Sensor Readings
void publishSensorData(){
  char sensordata_template[] = "{'message': 'sensor','hub_id': 'mac-addr','hub_name': 'Maggie','utc_timestamp': 'YYYY-MM-DD HH:MM:SS:ms','sensor_id': 'xxx[pin]','sensor_name': 'Tank water level','reading': %d,'units': 'inches'}";

  // Ultra-sonic
  char sensor_message[1000];
  long RangeInInches;
  RangeInInches = ultrasonic.MeasureInInches();
  sprintf(sensor_message, sensordata_template, RangeInInches);
  Serial.println(sensor_message);
  publishMessage(sensor_message);

  // Thermacouple
  Serial.print("Thermacouple reading=");
  Serial.println(thermocouple.readFahrenheit());

}

void publishHeraldMessage(){
  // IP addres, macaddress, device id, other
  char herald_message[1000];
  int rssi;
  rssi = -55;

  char hearld_template[] = "{'message': 'herald','hub_id': 'mac-addr','hub_name': 'Maggie','utc_timestamp': 'YYYY-MM-DD HH:MM:SS:ms','ip_add': 'x.x.x.x','rssi': %d}";
  sprintf(herald_message, hearld_template, rssi);
  Serial.println(herald_message);
  publishMessage(herald_message);
}


void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
