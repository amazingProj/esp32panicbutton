/*
   ....................... Includes section ...................
*/
// includes mqtt popular library
#include <PubSubClient.h>
#include "WiFi.h"
#include "WiFiMulti.h"
#include "stdio.h"
#include "String.h"

/***************** defines variables  ***********************/

//WIFI LOGIN
#ifndef STASSID
#define STASSID "benny" //CHANGE TO YOUR NETWORK NAME
#define STAPSK  "0585002913" //CHANGE TO YOUR NETWORK PASSWORD
#endif

#define DEBUG_MODE 1
#define LOOPER_DELAY 30000
#define PORT 3000
#define ACCESSPOINT_INFO_LENGTH 34

/*** define symbols ***/
#define RIGHT_PARENTHESIS "{"
#define LEFT_PARENTHESIS "}"
#define COLON_TOKEN ":"
#define COMMA ","

/*************************** global variables *********************/
//CODE----------------------------
const char* ssid     = STASSID;
const char* password = STAPSK;
char  buf[100];
char buf1[100];
char buf2[100];
char messageBuffer[1024];

//The broker and port are provided by http://www.mqtt−dashboard.com/
char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;
String mqttMessage = "";
int FROMStringIndex = 0;
int TOStringIndex = 0;
int messageSize = 0;
int headerOfMqttMessageLength = 0;
int partialMessageLength = 194;
int temp = 0;

//Replace these 3 with the strings of your choice
const char* mqtt_client_name = "ESPYS2111";
const char* mqtt_pub_topic = "/ys/testpub"; //The topic to which our client will publish
const char* mqtt_sub_topic = "/ys/testsub"; //The topic to which our client will subscribe

WiFiClient client;
PubSubClient mqttClient(client);

WiFiMulti WiFiMulti;
uint64_t messageTimestamp;

/*********************************** functions ******************************************/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received from: "); Serial.println(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println();

}

/**
   set up lifecycle function of esp32 device
   the function does:
   connect the device to wifi access point
   set up a socket.io connection
*/
void setup()
{
  Serial.begin(115200);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  connectWifi();
  delay(100);
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
  Serial.println("Setup done");
}

/**
   loop lifecycle function do it while the esp32 is connected to power
   the function does:
   send the rssi of each access point to server
*/
void loop()
{
  // gets the time now in miliseconds
  uint64_t now = millis();
  if (now - messageTimestamp > LOOPER_DELAY) {
    messageTimestamp = now;
    mqttMessage = RIGHT_PARENTHESIS;
    mqttMessage = mqttMessage + "numberOfAccessPoints" + COLON_TOKEN;
    int n = WiFi.scanNetworks();
    mqttMessage = mqttMessage + String(n) + COMMA;
    Serial.println("scan done");
    if (n == 0) {
      Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      mqttMessage = mqttMessage + "macAddress" + COLON_TOKEN;
      mqttMessage = mqttMessage + WiFi.macAddress() + COMMA;
      mqttMessage = mqttMessage + "Ssid" + COLON_TOKEN;
      mqttMessage = mqttMessage + WiFi.SSID() + COMMA;
      headerOfMqttMessageLength = mqttMessage.length();
      if (DEBUG_MODE) {
        Serial.printf("MAC address = %s", WiFi.softAPmacAddress().c_str());
        Serial.println(WiFi.macAddress());
      }
      for (int i = 0; i < n; ++i) {
        mqttMessage = mqttMessage + RIGHT_PARENTHESIS;

        mqttMessage = mqttMessage + "Bssid" + COLON_TOKEN + WiFi.BSSIDstr(i) + COMMA;

        mqttMessage = mqttMessage + "rssi" + COLON_TOKEN + String(WiFi.RSSI(i));

        mqttMessage = mqttMessage + LEFT_PARENTHESIS;

        if (i != n - 1) {
          mqttMessage = mqttMessage + COMMA;
        }

      }

      if (!mqttClient.connected()) {
        while (!mqttClient.connected()) {
          if (mqttClient.connect(mqtt_client_name)) {
            Serial.println("MQTT Connected!");
            mqttClient.subscribe(mqtt_sub_topic);
          }
          else {
            Serial.print(".");
          }
        }
      }
      mqttMessage = mqttMessage + LEFT_PARENTHESIS;
      //Serial.println(mqttMessage);
      messageSize = mqttMessage.length();
      partialMessageLength = 4 * ACCESSPOINT_INFO_LENGTH;
      bool firstTime = true;
      for (FROMStringIndex = 0; FROMStringIndex < messageSize; FROMStringIndex += partialMessageLength) {
       
        if (FROMStringIndex + partialMessageLength > messageSize) {
          TOStringIndex = messageSize;
        }
        else {
          TOStringIndex = FROMStringIndex + partialMessageLength;
          if(!firstTime){
            TOStringIndex += 4;
          }
        }

        if (firstTime){
          TOStringIndex += headerOfMqttMessageLength; 
          TOStringIndex += 3;
        }
        Serial.println(FROMStringIndex);
        Serial.println(TOStringIndex);
        Serial.println("\n");
        mqttMessage.substring(FROMStringIndex, TOStringIndex).toCharArray(messageBuffer, sizeof(messageBuffer));
        //Serial.println(messageBuffer);
        mqttClient.publish(mqtt_pub_topic, messageBuffer);
        if (!firstTime){
          FROMStringIndex += 4;  
        }
        
        if (firstTime){
          FROMStringIndex += headerOfMqttMessageLength;
          firstTime = false;
          FROMStringIndex += 3;
        }
        
       
      }

      Serial.println("Message published");
    }
    Serial.println("");
  }
}

/**
   connects the device to access point wifi function
   the function connects the esp32 to wifi.
   thus it can send requested to server
*/
void connectWifi() {
  //Connects ESP2866 Wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  //Wait until connection is established:
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //Wifi connected, print IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);
}
