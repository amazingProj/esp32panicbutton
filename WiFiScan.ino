#include <PubSubClient.h>
/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include "WiFiMulti.h"
#include "stdio.h"

#define DEBUG_MODE 1
#define LOOPER_DELAY 12000
#define PORT 3000

//WIFI LOGIN
#ifndef STASSID
#define STASSID "AndroidAPFE24" //CHANGE TO YOUR NETWORK NAME
#define STAPSK  "ehgo7879" //CHANGE TO YOUR NETWORK PASSWORD
#endif

//CODE----------------------------
const char* ssid     = STASSID;
const char* password = STAPSK;
char  buf[100];
char buf1[100];
char buf2[100];
//The broker and port are provided by http://www.mqtt−dashboard.com/
char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;


class Information {
  private:
    int macStrSize = 64;
  public:
    bool isAlarmedOn;
    int numberOfAccessPoints;
    char *macAddressDevice;

    Information(){
      isAlarmedOn = false;
      numberOfAccessPoints = 0;
      macAddressDevice = new Char[macStrSize];
    }
    
    Information(bool _isAlarmedOn, int _numberOfAccessPoints, char *_macAddress){
      isAlarmedOn = _isAlarmedOn;
      numberOfAccessPoints = _numberOfAccessPoints;
      macAddressDevice = _macAddress;
    }

    char *JsonFormat(){
      int size = 264;
      char *str = new Char[size];
      str = "{";
      str += isAlarmedOn.toString();
      str += String(numberOfAccessPoints);
      str += macAddressDevice;
    }
    
}

Information info = Information();

//Replace these 3 with the strings of your choice
const char* mqtt_client_name = "ESPYS2111";
const char* mqtt_pub_topic = "/ys/testpub"; //The topic to which our client will publish
const char* mqtt_sub_topic = "/ys/testsub"; //The topic to which our client will subscribe

WiFiClient client;
PubSubClient mqttClient(client);

WiFiMulti WiFiMulti;
uint64_t messageTimestamp;

void callback(char* topic, byte* payload, unsigned int length) {
   Serial.print("Message received from: "); Serial.println(topic);
   for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
   }
   Serial.println();
   Serial.println();
}

/**
 * set up lifecycle function of esp32 device
 * the function does:
 * connect the device to wifi access point 
 * set up a socket.io connection
 */
void setup()
{
    Serial.begin(115200);
    isAlarmedOn = false;
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
 * loop lifecycle function do it while the esp32 is connected to power
 * the function does:
 * send the rssi of each access point to server
 */
void loop()
{
  
  uint64_t now = millis();
  if(now - messageTimestamp > LOOPER_DELAY) {
    messageTimestamp = now;
    // Send event     
    int n = WiFi.scanNetworks();
    info.numberOfAccessPoints = n;
    if (DEBUG_MODE){
      Serial.printf("MAC address = %s", WiFi.softAPmacAddress().c_str());
      Serial.println(WiFi.macAddress());
    }
    
    info.macAddressDevice = WiFi.macAddress();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            
            WiFi.macAddress().toCharArray(buf2, sizeof(buf2));
            // send via mqtt the device's mac address
         
            
            WiFi.BSSIDstr(i).toCharArray(buf1, sizeof(buf1));
            // send the mac address of the access point via mqtt
            
            
            snprintf(buf, sizeof(buf), "%d",  WiFi.RSSI(i));
            // send the rssi from the access point via mqtt
            
        }
        
        if (!mqttClient.connected()){
            while (!mqttClient.connected()){
               if(mqttClient.connect(mqtt_client_name)){
                  Serial.println("MQTT Connected!");
                  mqttClient.subscribe(mqtt_sub_topic);
               }
               else{
                  Serial.print(".");
               }
            }
        }
        mqttClient.publish(mqtt_pub_topic, buf);
        Serial.println("Message published");
    }
    Serial.println("");
  }    
}

/**
 * connects the device to access point wifi function
 * the function connects the esp32 to wifi.
 * thus it can send requested to server 
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
