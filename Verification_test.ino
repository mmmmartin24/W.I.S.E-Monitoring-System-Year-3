#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <WifiClient.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

// Wi-Fi
const char *ssid = "AndroidAPCA5E";
const char *password = "Apahayo.";
WiFiClient wifiClient;

// MQTT
const char *mqtt_server = "192.168.132.190";
const char *data_topic = "data"; 
const char* mqtt_username = "siaiti"; 
const char* mqtt_password = "wololo"; 
const char *clientID = "RFID_Station_1"; 
PubSubClient client(mqtt_server, 1889, wifiClient);

#include <map>
#include <string>
std::map<int, std::string> fingerprintMap;


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(12,14);
#else
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myservo;
uint8_t id;
#define button_pin D4

String status = "0";
LiquidCrystal_I2C lcd(0x27,20,4); // addresss i2c = 0x27

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
  lcd.init();
  lcd.begin(20,4);      
  lcd.backlight(); 
  lcd.print("Connecting...");
  pinMode(button_pin,INPUT_PULLUP);
  setup_wifi(); 
  SPI.begin(); 
  client.setCallback(callback);
  myservo.attach(D7);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  Serial.println("connected");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  lcd.clear();

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

     // Make sure backlight is on
  
}

String readString() {
  String str = "";

  while (str.length() == 0) {
    while (!Serial.available());
    str = Serial.readStringUntil('\n');
  }
  return str;
}


uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert payload to string
  char payload_str[length + 1];
  strncpy(payload_str, (char*)payload, length);
  payload_str[length] = '\0';

  // Create a new string for the first two bytes
  char first_two_bytes[4];
  strncpy(first_two_bytes, payload_str, 3);
  first_two_bytes[3] = '\0';  // Null-terminate the new string

  // Convert payload to float
  float value = atof(first_two_bytes);

  // Round to one decimal place
  value = roundf(value * 10) / 10;

  if (strcmp(topic, "tempValue") == 0){
    Serial.print(value);
    lcd.setCursor(7, 1);
    lcd.print(value, 1);  // Print value with 1 decimal place
  }
  else if (strcmp(topic, "humValue") == 0){
    Serial.print(value);
    lcd.setCursor(10, 2); // Change the cursor position as needed
    lcd.print(value, 1);  // Print value with 1 decimal place
  }
  else if (strcmp(topic, "gasValue") == 0){
    Serial.print(value);
    lcd.setCursor(12, 0); // Change the cursor position as needed
    lcd.print(value, 1);  // Print value with 1 decimal place
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
        Serial.print("Subscribed!");
        client.subscribe("gasValue");
        client.subscribe("tempValue");
        client.subscribe("humValue"); 

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
  
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
   Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  if (finger.confidence > 100){
    status = String(finger.fingerID);
    client.publish("user", status.c_str());
    Serial.print("verified");
    lcd.setCursor(5, 3);
    lcd.print("VERIFIED  ");
    myservo.write(0);
    delay(2000);
    myservo.write(90);
    delay(10000);
    lcd.setCursor(5, 3);
    lcd.print("UNVERIFIED"); 
    client.publish("user", "0");
  }else{
    
  }
  
  delay(15);
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  lcd.setCursor(0, 0);
  lcd.print("Air Quality:"); 
  lcd.setCursor(0, 1);
  lcd.print("TEMP:");
  lcd.setCursor(0,2);
  lcd.print("HUMIDITY:");
  lcd.setCursor(0, 3);
  lcd.print("USER:"); 
  lcd.setCursor(5, 3);
  lcd.print("UNVERIFIED");

  if (digitalRead(button_pin) == HIGH){
      getFingerprintID();
      delay(50);  
    }else
  {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!  getFingerprintEnroll() );
  }
}
