/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

    the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
    Deep sleep mode until RESET pin is connected to a LOW signal 
    (for example pushbutton or magnetic reed switch)
  
mqtt_esp8266_test2 turn on led pin 2 = D4 and built in led
     and sleep for 555 seconds or 9 minutes
mqtt_esp8266_test3 turn on pin 4 = D2, which will eventually be set to FET eventually to keep ESP8266 on for 10 seconds
    which is enough time for mqtt message to be uploaded 
    
    On the NodeMCU, GPIO 16 is represented as D0 . If we take a look at the 
    pinout for the NodeMCU, we can see that ... You need to connect GPIO 16 to RST to
    wake up ( or reset ) the device when Deep-sleep is ove    
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>

// Update these with values suitable for your network.

const char* ssid = "dickoryParima";
const char* password = "GeorgeWashington";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const int delayTime=1000; // 5 sec delay between messages
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "nestor.dyn-o-saur.com";
// Topic base for all comms from this device.
// change Delivery to Deliver for testing
//#define TOPICBASE    "Home/LetterBox/Delivery"
#define TOPICBASE    "Home/LetterBox/Delivery"
#define TOPICBATTERY "Home/LetterBox/Delivery"
#define TOPICTIME    "Home/LetterBox/Time"
#define TOPICDEBUG    "Home/LetterBox/DebugLine"
long lastMsg = 0;
char msg[75];
int value = 0;
int ledPin = 4; // ESP8266 12-E pin 2 = D4 = built-in. pin 0 = D3, 12=D6, 13=D7, 14=D5  4=D2
// lolin pin 0=D3, 1=Tx, pin 2=D4 and built-in, pin 3=Rx,4=D2, 5=D1, 6=not avail,7=not avail,8=don't use, 9=not avail,10=SK, 11=don't use, 12=D6, 13= D7,14=D5, 15=D8, 16=don't use,17=don't use
// WiFi.config(ip, dns, gateway);
int D2pin=0;
int ledPinD1 = 5;
IPAddress ip(192, 168, 1, 155);
IPAddress dns(8,8,4,4);
IPAddress gateway(192, 168, 1, 1);
const int sensorPin = A0;    // select the input pin for the potentiometer
int relayPin=4; // pin 4 = D2, which will eventually be set to FET eventually to keep ESP8266 on


// --------------------------- begin void blinkit(int pin, int delay, int repeatNum)
void blinkit(int pin, int delayTime, int repeatNum){
  pinMode(pin, OUTPUT); // pin 2 = D4 = built-in D2=0
  for(int i = 0; i < repeatNum; i++){
      digitalWrite(pin, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(delayTime);                  // wait for a second
      digitalWrite(pin, LOW);    // turn the LED off by making the voltage LOW
      delay(delayTime);                  // wait for a second
  } // end for 
  delay(delayTime*5);   
} // end void blinkit(int pin, int delayTime, int repeatNum)
// --------------------------- end void blinkit(int pin, int delay, int repeatNum)

// ------------------------------- begin void setup_wifi()
void setup_wifi() {
// no wifi.conf took 12761 ms with fresh download, 5001 ms with reset
// with wifi.conf    11845  ms.................... 5001 ms with reset
// with wifi.conf    11768 ms with fresh download, 1001 ms with reset
  digitalWrite(relayPin, HIGH);   // turn the LED on (HIGH is the voltage level) see line 223
  Serial.print("relayPin: "); Serial.print(relayPin); Serial.println(" HIGH ");
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
//delay(2000);
  //WiFi.config(ip, dns, gateway); 
  WiFi.config(ip, dns, gateway);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  } // end while

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
//delay(2000);  
} // end void setup_wifi()
// ----------------------------------------------------end void setup_wifi()

// --- begin void callback(char* topic, byte* payload, unsigned int length)

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

} // end void callback(char* topic, byte* payload, unsigned int length)
// ----------------------------- end void callback(char* topic, byte* payload, unsigned int length)



// ------------------------------ begin void reconnect()
void reconnect() {
  // blinkit(int pin, int delayTime, int repeatNum)
  // pin 2 = D4 = built-in 0=D3
  //blinkit(D2pin, 100, 10);
  // Loop until we're reconnected
  while (!client.connected()) { 
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);Serial.print("reconnect() line 152 clientId: "); Serial.println(clientId);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected"); long now = millis();
      snprintf (msg, 75, "%ld", now);
      Serial.print("Publish milliseconds message: ");
      Serial.println(msg);
      client.publish(TOPICBASE,msg); // time since boot first
      Serial.println("17j ~/esp/mqtt_esp8266_test4-DS2");
      
      value=analogRead(sensorPin);
      snprintf (msg, 75, "%ld", value);
      Serial.print("Publish analog sensor message: ");
      Serial.println(msg);
      // client.publish("outTopic", msg);
      client.publish(TOPICBATTERY, msg); // battery next
      
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  } // end while (!client.connected())
  blinkit(D2pin, 100, 1);
  blinkLed(ledPinD1);
} // end void reconnect()

// ------------------------------------- end void reconnect()

// ------------------------ begin void setup()
void blinkLed(int pin){
  int delayTime = 400; // 100 ms
  pinMode(pin, OUTPUT); // pin 2 = D4 = built-in D2=0
  digitalWrite(pin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delayTime);                  // wait for a second
  digitalWrite(pin, LOW);    // turn the LED off by making the voltage LOW
  delay(delayTime);                  // wait for a second
}// end void blinkLed
void setup() {
  EEPROM.begin(512);
  pinMode(relayPin, OUTPUT); // pin 4 = D2, which will eventually be set to FET eventually to keep ESP8266 on
  //digitalWrite(relayPin, HIGH);
  //blinkit(relayPin,100,1);
  Serial.begin(115200);
  delay(35);
  Serial.println();
  Serial.println("17j ~/ESP_Obsolete/mqtt_esp8266_test4-DS2FlashAfterSendMQTT_D1OnAfterConnect");
  Serial.println("17j ~/Arduino/mqtt_esp8266_test4-DS2FlashAfterSendMQTT_D1OnAfterConnect");
  Serial.println("begin setup wifi");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
} // end void setup()

// ------------------------ end void setup()

// -------------------------- begin void loop() 

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //long now = millis();
  // if (now - lastMsg > delayTime) {
  //lastMsg = now;
  //value=analogRead(sensorPin);
  //snprintf (msg, 75, "%ld", now);
  //Serial.print("Publish message: ");
  //Serial.println(msg);
  // client.publish("outTopic", msg);
  //client.publish(TOPICBASE, msg);
  //snprintf (msg, 75, "%ld", value);
  //Serial.print("Publish message: ");
  //Serial.println(msg);
  // client.publish("outTopic", msg);
  //client.publish(TOPICBASE, msg);
    // void blinkit(int pin, int delayTime, int repeatNum)
    //blinkit(ledPin,5000,1);
    delay(75);
    //digitalWrite(relayPin, LOW);    // turn the LED off by making the voltage LOW see line 90
    //Serial.print("relayPin: "); Serial.print(relayPin); Serial.print(" LOW ");
   Serial.println("Sleep");// Turn the LED off by making the voltage HIGH
    /*  Your built-in led is connected to gpio16, so you are resetting their chip when you are configuring
      gpio16 as output. Try using another pin for an led. 
      https://github.com/esp8266/Arduino/issues/2930
    */  
    ESP.deepSleep(5000000,WAKE_RF_DEFAULT); //go to sleep for 5 second
    //ESP.deepSleep(60000000,WAKE_RF_DEFAULT); //go to sleep for 60 second
   // ESP.deepSleep(555000000,WAKE_RF_DEFAULT); //go to sleep for 555 second = 9 min
    delay(500);
  // }
} // end void loop() 
