/*
 * uMQTTBroker demo for Arduino
 * 
 * Minimal Demo: the program simply starts a broker and waits for any client to connect.
 */

#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"
#include "/home/fido/Arduino/credentials.h"
uMQTTBroker myBroker;
const char* programName = "17j ~/Arduino/uMQTTBrokerSample";

/*
 * Your WiFi config here
 
char ssid[] = "ssid";      // your network SSID (name)
char pass[] = "password"; // your network password
*/
/*
 * WiFi init stuff
 */
void startWiFiClient()
{
  Serial.println("Connecting to "+(String)ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void startWiFiAP()
{
  WiFi.softAP(ssid, pass);
  Serial.println("AP started");
  Serial.println("IP address: " + WiFi.softAPIP().toString());
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  Serial.println();
  Serial.println(programName);

  // Connect to a WiFi network
  startWiFiClient();

  // Or start the ESP as AP
//startWiFiAP();

  // Start the broker
  Serial.println("Starting MQTT broker");
  myBroker.init();
}

void loop()
{   
  // do anything here
  delay(1000);
}
