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
#include <RTCZero.h>
#include <Ultrasonic.h>
#include <ArduinoJson.h>
#include <max6675.h>
#include <rgb_lcd.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <utility/wifi_drv.h> //included with WiFiNINA

#include "arduino_secrets.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
#define DHTTYPE    DHT11     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);

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
void print2digits(int number);
void printTime();
void printDate();
void getDateTime(char* datetime);
void displaySensorData(float ec, float water_temp, float air_temp, float humidity);
void rainbowStep(int delaytime, int counter);
void setLEDColor(uint8_t R, uint8_t G, uint8_t B);

//RGB LED pin definitions for MKR WiFi 1010
#define GREEN_LED_pin   25
#define BLUE_LED_pin    27
#define RED_LED_pin     26

int loopCount = 0;


// MAX6675 Thermocouple settings and pin locations
// GND to Ground, VCC to 5V output
int ktcSO = 8; // SO to digital pin 9
int ktcCS = 9;  // CS to digital pin 9
int ktcCLK = 10; // SCK to digital pin 10

MAX6675 thermocouple(ktcCLK, ktcCS, ktcSO);

// TDS Sensor 
#define SERIAL Serial
#define sensorPin A0

// Real Time Clock use
RTCZero rtc;
const int GMT = 0; //change this to adapt it to your time zone

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

// WiFi globals
long rssi;
IPAddress ip; 

// Configuration parameters
const int message_interval = 5*60*1000; // time between sensor messages miliseconds
const char command_topic[] = "commands/gardenhub/maggie";
const char sensordata_topic[] = "sensordata/gardenhub/maggie";
const char hub_name[] = "Maggie";

// Some global state
WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

Ultrasonic ultrasonic(1);
unsigned long lastMillis = 0;
rgb_lcd lcd;

uint32_t delayMS;

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
  mqttClient.setId("Maggie");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onJsonMessageReceived);

  // Setup the number of rows and columns
  lcd.begin(16, 2);
  lcd.display();
  lcd.clear();
  lcd.print("It's Maggie");
  lcd.setCursor(0, 1);
  lcd.print("Getting setup..");

  // Initize our DHT - Humidity and Temp sensor
    // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  // Initialize onboard RGB LED 
  //set ESP32 wifi module RGB led pins to output
  WiFiDrv::pinMode(RED_LED_pin, OUTPUT);   //RED
  WiFiDrv::pinMode(GREEN_LED_pin, OUTPUT); //GREEN
  WiFiDrv::pinMode(BLUE_LED_pin, OUTPUT);  //BLUE

  //turn off ESP32 wifi module RGB led
  WiFiDrv::digitalWrite(RED_LED_pin, LOW);    //values are uint8_t
  WiFiDrv::digitalWrite(GREEN_LED_pin, LOW);
  WiFiDrv::digitalWrite(BLUE_LED_pin, LOW);

  //set ESP32 wifi module RGB led color
  WiFiDrv::analogWrite(RED_LED_pin, 0);     //Red off
  WiFiDrv::analogWrite(GREEN_LED_pin, 255); //Green to 100%
  WiFiDrv::analogWrite(BLUE_LED_pin, 0);    //Blue off

  //set ESP32 wifi module RGB led color with setLEDColor(R,G,B) function. Range is 0-255 for each color.
  setLEDColor(0,255,0); //Green to 100%
  delay(2000);

  setLEDColor(0,0,0); //All LEDs off
  delay(500);   
}

void loop() {
  rainbowStep(3, loopCount);
  loopCount++;

  if (WiFi.status() != WL_CONNECTED) {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cxing to WiFi");
    connectWiFi();
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    printMacAddress(mac);
    printWifiStatus();
    rtc.begin();
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("WiFi Connected!");
    digitalWrite( BLUE_LED_pin, HIGH );
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println("Cxing to AWS IoT"); 
    connectMQTT();
    publishHeraldMessage();
    //publishMessage("I am Maggie and I am allive");
    lcd.setCursor(0,1);
    lcd.println("Connected!");
     digitalWrite( RED_LED_pin, HIGH );
    // Lets publish our first set of data as well... sucks wiating
    publishSensorData();
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
  unsigned long epoch;
  epoch = WiFi.getTime();
  Serial.print("Epoch received: ");
  Serial.println(epoch);
  rtc.setEpoch(epoch);
  return epoch;
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
  char sensordata_template[] = "{'message': 'sensor', 'hub_id': 'mac-addr','hub_name': '%s','utc_timestamp': '%s','sensor_id': '%s','sensor_name': '%s','reading': %s,'units': '%s'}";
  long RangeInInches;
  float air_temp;
  float humidity;
  float ec;

  // Get the time to print
  // printTime();
  // printDate();
  char dt_timestamp[19];
  getDateTime(dt_timestamp);

  // Ultra-sonic
  char sensor_message_us[1000];
  char dist_str[5];
  RangeInInches = ultrasonic.MeasureInInches();
  sprintf(dist_str, "%d", RangeInInches);
  sprintf(sensor_message_us, sensordata_template, hub_name, dt_timestamp, "Ultrasonic[D1]", "Tank water level", dist_str, "inches");
  //Serial.println(sensor_message_us);
  publishMessage(sensor_message_us);


  // Thermacouple
  // Serial.print("Thermacouple reading=");
  // Serial.println(thermocouple.readFahrenheit());


  // Humidity and Temperature
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    //Serial.print(F("Temperature: "));
    //Serial.print(event.temperature);
    //Serial.println(F("°C"));
    char sensor_message_dht[1000];
    char temp_str[6];
    air_temp = event.temperature * 9/5 + 32;
    sprintf(temp_str, "%6.2f", event.temperature);
    sprintf(sensor_message_dht, sensordata_template, "Maggie", dt_timestamp, "DHT[D2]", "Ambient Temperature", temp_str, "°C");
    //Serial.println(sensor_message_dht);
    publishMessage(sensor_message_dht);
  }


  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    // Serial.print(F("Humidity: "));
    // Serial.print(event.relative_humidity);
    // Serial.println(F("%"));

    char sensor_message_dht[1000];
    char hum_str[6];
    humidity = event.relative_humidity;
    sprintf(hum_str, "%6.2f", event.relative_humidity);
    sprintf(sensor_message_dht, sensordata_template, "Maggie", dt_timestamp, "DHT[D2]", "Ambient Humidity", hum_str, "%");
    //Serial.println(sensor_message_dht);
    publishMessage(sensor_message_dht);
  } 

  // Total Disolved Solids
  int sensorValue = 0;
  float tdsValue = 0;
  float Voltage = 0;
  float ecValue = 0;
  char sensor_message_ec[1000];
  char ec_str[6];

  sensorValue = analogRead(sensorPin);
  Voltage = sensorValue*5/1024.0; //Convert analog reading to Voltage
  tdsValue=(133.42*Voltage*Voltage*Voltage - 255.86*Voltage*Voltage + 857.39*Voltage)*0.5; //Convert voltage value to TDS value
  // Serial.print("TDS Value = "); 
  // Serial.print(tdsValue);
  // Serial.println(" ppm");
  ecValue = tdsValue * 2 / 1000;
  ec = ecValue;

  Serial.print("EC Value = "); 
  Serial.print(ecValue);
  Serial.println(" mS/cm");

  sprintf(ec_str, "%6.2f", ecValue);
  sprintf(sensor_message_ec, sensordata_template, "Maggie", dt_timestamp, "TDS[A0]", "Water EC", ec_str, "mS/cm");
  //Serial.println(sensor_message_ec);
  publishMessage(sensor_message_ec);
  displaySensorData(ec, 69.0, air_temp, humidity);
}

void publishHeraldMessage(){
  // IP addres, macaddress, device id, other
  char herald_message[1000];

  char dt_timestamp[19];
  getDateTime(dt_timestamp);

  char hearld_template[] = "{'message': 'herald','hub_id': 'mac-addr','hub_name': '%s','utc_timestamp': '%s','ip_add': 'x.x.x.x','rssi': %d}";
  sprintf(herald_message, hearld_template, hub_name, dt_timestamp, rssi);
  //Serial.println(herald_message);
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
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printTime()
{
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

void printDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());

  Serial.print(" ");
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void getDateTime(char* datetime)
{
  // char datetime[17];
  sprintf(datetime, "%d-%d-%d %d:%d:%d", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
  Serial.print("DateTime string: ");
  Serial.print(datetime);
  Serial.println();
}

void displaySensorData(float ec, float water_temp, float air_temp, float humidity)
{
  // EC=3.6 WT=72
  // AT=76  RH=85
  char line1[16];
  char line2[16];
  sprintf(line1, "EC=%3.1f  WT=%2.0f", ec, water_temp);
  sprintf(line2, "AT=%2.0f   RH=%2.0f", air_temp, humidity);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}

void setLEDColor(uint8_t R, uint8_t G, uint8_t B){
  //R, G, and B values should not exceed 255 or be lower than 0.
  
  //set ESP32 wifi module RGB led color
  WiFiDrv::analogWrite(RED_LED_pin, R); //Red
  WiFiDrv::analogWrite(GREEN_LED_pin, G); //Green
  WiFiDrv::analogWrite(BLUE_LED_pin, B);  //Blue
}

void rainbowStep(int delaytime, int counter){

    //increase red
    for(int i=0; i<255; i++){
        WiFiDrv::analogWrite(RED_LED_pin, i); //Red
        delay(delaytime);
    }
    
    //if this is 2nd run or higher, then decrease blue.
    if( counter > 0){
        loopCount = 5; //reset global to low number so memory never blows up

        //decrease blue
        for(int i=254; i>=0; i--){
            WiFiDrv::analogWrite(BLUE_LED_pin, i);  //Blue
            delay(delaytime);
        }
    }
    
    //increase green
    for(int i=0; i<255; i++){
        WiFiDrv::analogWrite(GREEN_LED_pin, i); //Green
        delay(delaytime);
    }
    
    //decrease red
    for(int i=254; i>=0; i--){
        WiFiDrv::analogWrite(RED_LED_pin, i);
        delay(delaytime);
    }

    //increase blue
    for(int i=0; i<255; i++){
        WiFiDrv::analogWrite(BLUE_LED_pin, i);
        delay(delaytime);
    }
    
    //decrease green
    for(int i=254; i>=0; i--){
        WiFiDrv::analogWrite(GREEN_LED_pin, i);
        delay(delaytime);
    }
}