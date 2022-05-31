#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #else
 #include <WiFi.h>  
#endif

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

StaticJsonDocument<1596> doc;
JsonObject wifiScanData;

//for Button press check
#define BUTTON_PIN 21 // GIOP21 pin connected to button
#define ONBOARD_LED  2
// Variables will change:
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin
//Define resistors
#define R2 100
#define R3 10
#define VOLTAGE_OUT(Vin) (((Vin) * R3) / (R2 + R3))
//ESP32 0% and 100%
#define VOLTAGE_MAX 4200
#define VOLTAGE_MIN 3300
#define ADC_REFERENCE 1100
#define VOLTAGE_TO_ADC(in) ((ADC_REFERENCE * (in)) / 4096)
#define BATTERY_MAX_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MAX))
#define BATTERY_MIN_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MIN))



// global variables, change them to change code behavior
#define LOOPER_DELAY 10000

#define ACCESSPOINT_INFO_LENGTH 34
// how much access point per one message section
#define NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE 10

//---- WiFi settings
const char* ssid = "benny";
const char* password = "0585002913";

//---- MQTT Broker settings
// replace with your broker url
const char* mqttServer = "712d6a94edd544ddac8b5c44600f18d3.s1.eu.hivemq.cloud"; 
const char* mqttUsername = "Esp32";
const char* mqttPassword = "Esp32Asaf";
const int mqttPort = 8883;
//The topic to which our client will publish
const char* mqtt_pub_topic = "users/wifi/scan"; 
//The topic to which our client will subscribe
const char* mqtt_sub_topic = "users/acknowledgement"; 

WiFiClientSecure espClient;   
//WiFiClient espClient;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE  (1024)
char msg[MSG_BUFFER_SIZE];
String mqttString;
int stringLength = 0;
String fixedString;
String macAddressString;

// for task scheduling
uint64_t messageTimestamp;




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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";   
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
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

//================================================ setup ================================================
void setup() 
{
  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else  
    // for the ESP32 

    // enable this line and the the "certificate" code for secure connection
    espClient.setCACert(root_ca);      
  #endif
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

//===========================================button press setup ================================================
void setup() {
  Serial.begin(9600);
  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ONBOARD_LED,OUTPUT); 
}


//================================================ loop ================================================
void loop() 
{
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
    } 
    else {
      Serial.print(n);
      Serial.println(" networks found");

      macAddressString = WiFi.macAddress();
      // the percent of the battery here
      doc["Battery"] = 100;
      doc["NumberOfAccessPoints"] = n;
      doc["MacAddress"] = macAddressString;
      doc["WifiSsid"] = WiFi.SSID();
     
      /******************* save all the wifi access point scans in the message string ****************/
      
      for (int i = 0; i < n; ++i) {
        wifiScanData = doc.createNestedObject(String(i + 1));

        wifiScanData["EspMacAddress"] = WiFi.macAddress();

        wifiScanData["Bssid"] = WiFi.BSSIDstr(i);

        wifiScanData["Rssi"] = WiFi.RSSI(i);

        wifiScanData["Ssid"] = WiFi.SSID(i);

      }

      mqttString = doc.as<String>();
      stringLength = mqttString.length();
      int msgSize = stringLength / NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE;
      int counter = 0;
      for(int j = 0; j < NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE - 1; ++j){
         fixedString = String(j + 1) + "," + NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE + "," + macAddressString + "  " + mqttString.substring(counter, counter + msgSize);
         fixedString.toCharArray(msg, MSG_BUFFER_SIZE);
        
         client.publish(mqtt_pub_topic, msg);

         counter += msgSize;
      }

      fixedString = String(NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE) + "," + NUMBER_OF_PARTIAL_OF_TOTAL_MESSAGE + "," + macAddressString + "  " + mqttString.substring(counter, stringLength);
      fixedString.toCharArray(msg, MSG_BUFFER_SIZE);
        
      client.publish(mqtt_pub_topic, msg);
 
      Serial.println("Message published:\n" + mqttString + "\n" + "String size is: " + String(mqttString.length()));

      doc.clear();
    }
    Serial.println("");
  }
  
}
//================================================ loop to check if was pressed and battary voltage================================================
void loop() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH)
    digitalWrite(ONBOARD_LED,HIGH);
  // save the last state
  lastState = currentState;
  int calc_battery_percentage(int adc)
  {
    int battery_percentage = 100 * (adc - BATTERY_MIN_ADC) / (BATTERY_MAX_ADC - BATTERY_MIN_ADC);

    if (battery_percentage < 0)
        battery_percentage = 0;
    if (battery_percentage > 100)
        battery_percentage = 100;

    return battery_percentage;
  }
    
}
 
//=======================================  
// This void is called every time we have a message from the broker

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; ++i){
    incommingMessage += (char)payload[i];
  }
  
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
}
