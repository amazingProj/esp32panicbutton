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
#define SOCKETIO "http://192.168.112.23:3000" //CHANGE TO THE IP OF THE SOCKET IO SERVER


//WIFI LOGIN
#ifndef STASSID
#define STASSID "AndroidAPFE24" //CHANGE TO YOUR NETWORK NAME
#define STAPSK  "ehgo7879" //CHANGE TO YOUR NETWORK PASSWORD
#endif

//CODE----------------------------
const char* ssid     = STASSID;
const char* password = STAPSK;
char  buf[100];

SocketIoClient socket;
WiFiMulti WiFiMulti;

void setup()
{
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    connectWifi();
    delay(100);
    
    socket.begin(SOCKETIO);
    Serial.println("Setup done");
    socket.emit("esp32wifi-info", "hdkdkkjhjddj");
}

void loop()
{
    //Run Socket
    socket.loop();
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            snprintf(buf, sizeof(buf), "%d",  WiFi.RSSI(i));
            Serial.print(buf);
            socket.emit("esp32wifi-info", buf);
        }
    }
    Serial.println("");

    // Wait a bit before scanning again
    delay(LOOPER_DELAY);
}


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