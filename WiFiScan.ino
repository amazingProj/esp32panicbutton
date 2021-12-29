#include <SocketIoClient.h>

/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include "WiFiMulti.h"
#include "SocketIoClient.h"
#include "stdio.h"

#define LOOPER_DELAY 5000
#define SOCKETIO "192.168.112.23" //CHANGE TO THE IP OF THE SOCKET IO SERVER
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

SocketIoClient socketIO;
WiFiMulti WiFiMulti;
uint64_t messageTimestamp;

/**
 * set up lifecycle function of esp32 device
 * the function does:
 * connect the device to wifi access point 
 * set up a socket.io connection
 */
void setup()
{
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    connectWifi();
    delay(100);
    
    socketIO.begin("192.168.167.23", 3000);
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
  if(now - messageTimestamp > 12000) {
    messageTimestamp = now;
    // Send event     
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            Serial.printf("MAC address = %s", WiFi.softAPmacAddress().c_str());
            Serial.println(WiFi.macAddress());
            socket.emit("esp32wifi-info", WiFi.BSSIDstr(i));
            snprintf(buf, sizeof(buf), "%d",  WiFi.RSSI(i));
            Serial.print(buf);
            socket.emit("esp32wifi-info", buf);
        }
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
