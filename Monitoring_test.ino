//IMPORTS
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Bonezegei_DHT11.h>
#include "HX711.h"
#define DHT11_PIN D6  // Changed to 12 for NodeMCU D6 pin
#define BUZZER_PIN D2
#define DO_PIN D7
#define AO_PIN A0  // esp8266's pin connected to AO pin of the MQ2 sensor
int state = 0;
Bonezegei_DHT11 dht(DHT11_PIN);  // Changed to DHT11_PIN

// Wi-Fi
const char *ssid = "AndroidAPCA5E";
const char *password = "Apahayo.";
WiFiClient wifiClient;

// MQTT
const char *mqtt_server = "192.168.132.190";
const char *data_topic = "testing";
const char* mqtt_username = "siaiti"; 
const char* mqtt_password = "wololo"; 
const char *clientID = "RFID_Station_1"; 
PubSubClient client(mqtt_server, 1889, wifiClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_mqtt() {
  while (! client.connect(clientID)) Serial.println("Connection to MQTT Broker failed…");
  Serial.println("Connected to MQTT Broker!");
}

void setup() {
  Serial.begin(9600);  
  setup_wifi();                                         // Initialize serial communications with the PC
  SPI.begin();  
  client.setCallback(callback);
  pinMode(BUZZER_PIN, OUTPUT); //buzzer
  dht.begin();
  Serial.println("Warming up the MQ2 sensor");
  pinMode(DO_PIN, INPUT);
  delay(20000);  // wait for the MQ2 to warm up
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  if ((char)payload[0] == '1')  {
    tone(BUZZER_PIN, 1000);  // Start sound at 1000Hz
    Serial.println("BUZZER ON");
    delay(2000);
    noTone(BUZZER_PIN);
  } else{
    noTone(BUZZER_PIN);  // Stop sound
    Serial.println("BUZZER OFF");
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.subscribe("buzzer"); 
      Serial.println("connected");
      // client.publish("rfid_status", "hello world");
      // ... and resubscribe
      // client.subscribe("data");
      // client.publish("rfid_status", "1");

    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);

    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(1000);
  int gasValue = analogRead(AO_PIN);
  Serial.print("MQ2 sensor AO value: ");
  Serial.println(gasValue);
  String gasValuestr = String(gasValue);
  client.publish("gasValue", gasValuestr.c_str());
  Serial.println("gasValue sent!");

  if (dht.getData()) {                         // get All data from DHT11
      float tempDeg = dht.getTemperature(); 
      float humidity = dht.getHumidity();        // get humidity data from DHT11
      String tempDegStr = String(tempDeg);
      String humidityStr = String(humidity);
      client.publish("tempValue",tempDegStr.c_str());
      client.publish("humValue",humidityStr.c_str());
      Serial.println("tempValue sent!");
      Serial.println("humValue sent!");
      String strTemp  = "Temp: ";
      strTemp += tempDeg;
      strTemp += "°C  ";
      
      String strHumidity = "Humidity: ";
      strHumidity += humidity;
      strHumidity += "%";

      Serial.println(strTemp.c_str());
      Serial.println(strHumidity.c_str()); 



  delay(1000);

}}

