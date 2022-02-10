/*
 * Virtuino MQTT getting started example
 * Broker: HiveMQ (Secure connection)
 * Supported boards: ESP8266 / ESP32 
 */

#ifdef ESP8266
 #include <ESP8266WiFi.h>  // Pins for board ESP8266 Wemos-NodeMCU
 #else
 #include <WiFi.h>  
#endif
 
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// 0 is false and 1 is true
#define DEBUG_MODE 0
// global variables, change them to change code behavior
#define LOOPER_DELAY 10000
#define PORT 3000
#define ACCESSPOINT_INFO_LENGTH 34
// how much access point per one message section
#define ACCESS_POINT_SENT_PER_MESSAGE 4

/*** define symbols ***/
#define RIGHT_PARENTHESIS "{"
#define LEFT_PARENTHESIS "}"
#define COLON_TOKEN ":"
#define COMMA ","

//---- WiFi settings
const char* ssid = "benny";
const char* password = "0585002913";

//---- MQTT Broker settings
// replace with your broker url
const char* mqtt_server = "712d6a94edd544ddac8b5c44600f18d3.s1.eu.hivemq.cloud"; 
const char* mqtt_username = "Esp32";
const char* mqtt_password = "Esp32Asaf";
const int mqtt_port = 8883;
/// buffers in use in the code 
char messageBuffer[1024];
 

WiFiClientSecure espClient;   // for no secure connection use WiFiClient instead of WiFiClientSecure 
//WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

// belongs to the section that make the message in json format
String mqttMessage = "";
int FROMStringIndex = 0;
int TOStringIndex = 0;
int messageSize = 0;
int headerOfMqttMessageLength = 0;
int partialMessageLength = 194;
int temp = 0;
int commaNumber = ACCESS_POINT_SENT_PER_MESSAGE;

#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];


// for task scheduling
uint64_t messageTimestamp;

//The topic to which our client will publish
const char* mqtt_pub_topic = "users/wifi/scan"; 
//The topic to which our client will subscribe
const char* mqtt_sub_topic = "/ys/testsub"; 


static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


//==========================================
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}


//=====================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";   
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
       // Wait 5 seconds before retrying
      Serial.println(" try again in 5 seconds");  
      delay(5000);
    }
  }
}

//================================================ setup
//================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else   // for the ESP32
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}


//================================================ loop
//================================================
void loop() {

  if (!client.connected()) reconnect();
  //client.loop();

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
        mqttMessage = mqttMessage + String(i + 1) + COLON_TOKEN;
        
        mqttMessage = mqttMessage + RIGHT_PARENTHESIS;

        mqttMessage = mqttMessage + "Bssid" + COLON_TOKEN + WiFi.BSSIDstr(i) + COMMA;

        mqttMessage = mqttMessage + "rssi" + COLON_TOKEN + String(WiFi.RSSI(i));

        mqttMessage = mqttMessage + LEFT_PARENTHESIS;
        // if this is not the last access point put a comma between two differenet access points
        if (i != n - 1) {
          mqttMessage = mqttMessage + COMMA;
        }
      }
      
      mqttMessage = mqttMessage + LEFT_PARENTHESIS;
      
      messageSize = mqttMessage.length();
      partialMessageLength = ACCESS_POINT_SENT_PER_MESSAGE * ACCESSPOINT_INFO_LENGTH;
      bool firstTime = true;

      //Serial.println(mqttMessage);
      
      for (FROMStringIndex = 0; FROMStringIndex < messageSize; FROMStringIndex += partialMessageLength) {
       
        if (FROMStringIndex + partialMessageLength > messageSize) {
          TOStringIndex = messageSize;
        }
        else {
          TOStringIndex = FROMStringIndex + partialMessageLength;
          TOStringIndex += ACCESS_POINT_SENT_PER_MESSAGE * 2;
        }

        if (firstTime){
          TOStringIndex += headerOfMqttMessageLength; 
          
        }
        
        TOStringIndex += commaNumber;

        Serial.println(mqttMessage);
       
        mqttMessage.substring(FROMStringIndex, TOStringIndex).toCharArray(messageBuffer, sizeof(messageBuffer));
        
        client.publish(mqtt_pub_topic, messageBuffer);
     
        
        if (firstTime){
          FROMStringIndex += headerOfMqttMessageLength;
          
          firstTime = false;
        }
        
        FROMStringIndex += commaNumber;
        FROMStringIndex += ACCESS_POINT_SENT_PER_MESSAGE * 2;
      }

      Serial.println("Message published");
    }
    Serial.println("");
  }
  
}

//=======================================  
// This void is called every time we have a message from the broker

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
  
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
}
