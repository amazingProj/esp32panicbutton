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

//WIFI LOGIN information
#ifndef STASSID
#define STASSID "benny" //CHANGE TO YOUR NETWORK NAME
#define STAPSK  "0585002913" //CHANGE TO YOUR NETWORK PASSWORD
#endif

// 0 is false and 1 is true
#define DEBUG_MODE 0

// global variables, change them to change code behavior
#define LOOPER_DELAY 30000
#define PORT 3000
#define ACCESSPOINT_INFO_LENGTH 34
// how much access point per one message section
#define ACCESS_POINT_SENT_PER_MESSAGE 3


/*** define symbols ***/
#define RIGHT_PARENTHESIS "{"
#define LEFT_PARENTHESIS "}"
#define COLON_TOKEN ":"
#define COMMA ","


/*************************** global variables *********************/
//CODE----------------------------
// set the wifi ssid and password
const char* ssid     = STASSID;
const char* password = STAPSK;


/// buffers in use in the code 
char messageBuffer[1024];

//The broker and port are provided by http://www.mqtt−dashboard.com/
// mqtt variables
char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;
//Replace these 3 with the strings of your choice
const char* mqtt_client_name = "ESP32Client";
//The topic to which our client will publish
const char* mqtt_pub_topic = "/ys/testpub"; 
//The topic to which our client will subscribe
const char* mqtt_sub_topic = "/ys/testsub"; 




// belongs to the section that make the message in json format
String mqttMessage = "";
int FROMStringIndex = 0;
int TOStringIndex = 0;
int messageSize = 0;
int headerOfMqttMessageLength = 0;
int partialMessageLength = 194;
int temp = 0;
int commaNumber = ACCESS_POINT_SENT_PER_MESSAGE;

// for wifi scanning
WiFiClient client;

// for mqtt messaging
PubSubClient mqttClient(client);

// for connecting to wifi
WiFiMulti WiFiMulti;

// for task scheduling
uint64_t messageTimestamp;

/*********************************** functions ******************************************/

/**
 * calls back some by reference messages from the server
 * @param topic - a pointer to char array
 *        payload - a pointer to bytee array
 *        length - an int represents length of the payload
 */
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
   send the rssi of each access point to server in json format
*/
void loop()
{
  // gets the time now in miliseconds
  uint64_t now = millis();
  if (now - messageTimestamp > LOOPER_DELAY) {
    messageTimestamp = now;
    
    int n = WiFi.scanNetworks();
    
    Serial.println("scan done");
    if (n == 0) {
      Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      
      /***** set the initials values of the message ******/
      mqttMessage = RIGHT_PARENTHESIS;
      mqttMessage = mqttMessage + "numberOfAccessPoints" + COLON_TOKEN;
      mqttMessage = mqttMessage + String(n) + COMMA;
      mqttMessage = mqttMessage + "macAddress" + COLON_TOKEN;
      mqttMessage = mqttMessage + WiFi.macAddress() + COMMA;
      mqttMessage = mqttMessage + "Ssid" + COLON_TOKEN;
      mqttMessage = mqttMessage + WiFi.SSID() + COMMA;

      // save the length of the string (see above ) 
      headerOfMqttMessageLength = mqttMessage.length();
      
      if (DEBUG_MODE) {
        //Serial.printf("MAC address = %s", WiFi.softAPmacAddress().c_str());
        Serial.println(WiFi.macAddress());
      }

      /******************* save all the wifi access point scans in the message string ****************/
      for (int i = 0; i < n; ++i) {
        mqttMessage = mqttMessage + RIGHT_PARENTHESIS;

        mqttMessage = mqttMessage + "Bssid" + COLON_TOKEN + WiFi.BSSIDstr(i) + COMMA;

        mqttMessage = mqttMessage + "rssi" + COLON_TOKEN + String(WiFi.RSSI(i));

        mqttMessage = mqttMessage + LEFT_PARENTHESIS;
        // if this is not the last access point put a comma between two differenet access points
        if (i != n - 1) {
          mqttMessage = mqttMessage + COMMA;
        }

      }
      // make sure that it is connected to mqtt server
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
      
      messageSize = mqttMessage.length();
      partialMessageLength = ACCESS_POINT_SENT_PER_MESSAGE * ACCESSPOINT_INFO_LENGTH;
      bool firstTime = true;
      for (FROMStringIndex = 0; FROMStringIndex < messageSize; FROMStringIndex += partialMessageLength) {
       
        if (FROMStringIndex + partialMessageLength > messageSize) {
          TOStringIndex = messageSize;
        }
        else {
          TOStringIndex = FROMStringIndex + partialMessageLength;
         
        }

        if (firstTime){
          TOStringIndex += headerOfMqttMessageLength; 
          
        }
        
        TOStringIndex += commaNumber;
       
        mqttMessage.substring(FROMStringIndex, TOStringIndex).toCharArray(messageBuffer, sizeof(messageBuffer));
        
        mqttClient.publish(mqtt_pub_topic, messageBuffer);
     
        
        if (firstTime){
          FROMStringIndex += headerOfMqttMessageLength;
          
          firstTime = false;
        }
        
        FROMStringIndex += commaNumber;
       
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
