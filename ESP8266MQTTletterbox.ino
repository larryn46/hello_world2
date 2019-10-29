// amica node mcu 0=D3 1=none 2=D4=blue_led 5=D1 4=D2 don't ground D4 during bootload
// esp8266 feather huzzah 0=red builtin, 2=blue led
// lolin pin 0=D3, 1=T3=misox, pin 2=D4 and built-in, pin 3=Rx,4=D2, 5=D1, 6=not avail,
// lolin pin 7=not avail,8=don't use, 9=not avail,10=SK, 11=don't use, 12=D6, 
// lolin pin 13= D7,14=D5, 15=D8, 16=don't use=D0 ,17=don't use
// 12-E pin 2 = D4 = built-in. pin 0 = D3, 12=D6, 13=D7, 14=D5
#include <DHT.h>
#include "/home/fido/Arduino/credentials.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// ---------------------------------from ESP8266_Gmail_Sender
#include "Gsender.h"

#pragma region Globals
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals
// ---------------------------------from ESP8266_Gmail_Sender

WiFiClient espClient;
//const char* ssid = ".......";
//const char* password = "..........";

PubSubClient PSclient(espClient);
const char* mqtt_server = "192.168.1.94"; // const char* mqtt_server = "192.168.1.216";
const char* programName = "ESP8266MQTTletterbox";
// Topic base for all comms from this device.
#define TOPICBASE "Home/LetterBox/"

// Variables used for temperature & humidity sensing.
#define TEMP_PIN 0
DHT dhtSensor(TEMP_PIN, DHT11);
unsigned long confTempDelay = 10000;    // Default temperature publish delay. 10 sec.
unsigned long LastTempMillis = 0;       // Stores the last millis() for determining update delay.
float TempValue;
float HumidValue;
float HindexValue;

// Variables used for lux sensing.
Adafruit_TSL2561_Unified LuxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 42);
sensor_t LuxSensorInfo;
unsigned long confLuxDelay = 10000;    // Default temperature publish delay, (0 to disable).
unsigned long LastLuxMillis = 0;       // Stores the last millis() for determining update delay.
float LuxValue;

// Variables used for when someone delivers mail to the mailbox.
#define DELIVER_PIN 14
unsigned long confDeliverDelay = 5000;  // Default time to wait to assume the next mail delivery event, (0 to disable).
unsigned long DeliverLastMillis = 0;    // Stores the last millis() when waiting for the next mail delivery event.
boolean DeliverPubFlag = 0;             // Used to keep PSclient.publish out of int handlers.
int DeliverCount = 0;

// Variables used for when someone checks the mailbox for mail.
#define CHECK_PIN 12
#define LAZY_GND 13                     // Instead of having to use additional wires.
unsigned long confCheckDelay = 1000;    // Default time to wait for mailbox check debounce, (0 to disable).
unsigned long CheckLastMillis = 0;      // Stores the last millis() when waiting for the mailbox check debounce.
boolean CheckPubFlag = 0;               // Used to keep PSclient.publish out of int handlers.
int CheckCount = 0;

// ---------------------------------from ESP8266_Gmail_Sender
  
// ---------------------------------from ESP8266_Gmail_Sender
void  ICACHE_RAM_ATTR DeliverFunc();
void  ICACHE_RAM_ATTR CheckFunc();
void  WiFiConnect();
void  ICACHE_RAM_ATTR callback(char* topic, byte* payload, unsigned int length);

void setup() // --------------------------------------------------------------- void setup()
{
	Serial.begin(115200);
  delay(10);
  Serial.println("\n 17j ~/ESP_Obsolete/MQTTletterbox-master/MQTTletterbox ");
  Serial.println("\n 17j ~/Arduino/ESP8266MQTTletterbox ");
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, LOW);

	pinMode(DELIVER_PIN, INPUT);
	attachInterrupt(DELIVER_PIN, DeliverFunc, FALLING);

  pinMode(LAZY_GND, OUTPUT);
  digitalWrite(LAZY_GND, LOW);
	pinMode(CHECK_PIN, INPUT_PULLUP);
	attachInterrupt(CHECK_PIN, CheckFunc, FALLING);

	
	WiFiConnect();
	PSclient.setServer(mqtt_server, 1883);
	PSclient.setCallback(callback);

  dhtSensor.begin();

  if (!LuxSensor.begin())
  {
    Serial.println("Lux Sensor not found line 98");
    Publish((char *)"Lux", (char *)"DEAD");
  }
  else
  {
    LuxSensor.enableAutoRange(true);
    LuxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
    Serial.println("Lux Sensor found line 109");
  }

  
} // ---------------------------------------------- end void setup()

// -------------------------------------------------begin void loop()

void loop()
{ //Serial.print("Millis"); Serial.println(millis());
  
	if (!PSclient.connected()) //  PubSubClient PSclient(espClient);
	{
		Serial.println("PubSubClient PSclient not connected line 122, reconnect");
		reconnect();
    
	}
  //Serial.println("PSclient connected line 126"); 
	PSclient.loop();

  // Publish - someone has put mail in the box.
	if (DeliverPubFlag)
	{
    PublishInt((char *)"Deliver", DeliverCount); Serial.println("Mail delivered");
    // remove slashes from next line and comment out previous line for Production
    //PublishInt((char *)"Delivery", DeliverCount); Serial.println("Mail delivered");
		DeliverPubFlag = 0;                           Serial.println(DeliverCount);
	}

  // Publish - someone has checked the box for mail.
  if (CheckPubFlag)
	{
    PublishInt((char *)"Check", 1); Serial.println("Mailbox opened");
		CheckPubFlag = 0;               Serial.println();
	}

  if (confLuxDelay && (millis() - LastLuxMillis > confLuxDelay))
  {
    LastLuxMillis = millis();
    sensors_event_t event;
    LuxSensor.getEvent(&event);
 
    if (event.light)
    {
      LuxValue = event.light;               Serial.print("Lux = ");
      PublishFloat((char *)"Lux", LuxValue);Serial.println(LuxValue);
    }
    else
      Publish((char *)"Lux", (char *)"OL");
  }

	if (confTempDelay && (millis() - LastTempMillis > confTempDelay))
	{
		LastTempMillis = millis();

		TempValue = dhtSensor.readTemperature();        Serial.print("Mailbox Temp = ");
    PublishFloat((char *)"Temperature", TempValue); Serial.println(TempValue);

		HumidValue = dhtSensor.readHumidity();        Serial.print("Mailbox humidity = ");
    PublishFloat((char *)"Humidity", HumidValue); Serial.println(HumidValue);

		HindexValue = dhtSensor.computeHeatIndex(TempValue, HumidValue, false); Serial.print("Humidity Index = ");
    PublishFloat((char *)"HeatIndex", HindexValue);                         Serial.println(HindexValue);Serial.println();
	}
}

// -------------------------------------------------------------- end void loop()

// -------------------------------------------------------------- void Publish(char *Topic, char *Message)
// concat Topic to TOPICBASE, and publish whatever is in Message
void Publish(char *Topic, char *Message)
{
  char TopicBase[80] = TOPICBASE;

  strcat(TopicBase, Topic);
  PSclient.publish(TopicBase, Message);
}
// --------------------------------------------------------- end void Publish(char *Topic, char *Message)
//  concat Topic to TOPICBASE and changes Value to alpha if alphanumeric
void PublishInt(char *Topic, int Value)
{
  char TopicBase[80] = TOPICBASE;
  char Message[10] = "NULL";

  if (!isnan(Value))
    itoa(Value, Message, 10);

  strcat(TopicBase, Topic);
  PSclient.publish(TopicBase, Message);
}


void PublishFloat(char *Topic, float Value)
{
  char TopicBase[80] = TOPICBASE;
  char Message[10] = "NULL";

  if (!isnan(Value))
    dtostrf(Value, 5, 2, Message);

  strcat(TopicBase, Topic);
  PSclient.publish(TopicBase, Message);
}


void reconnect() // ------------------------------------------------------------------------- void reconnect()
{
	// Loop until we're reconnected
	while (!PSclient.connected())
	{
		Serial.println("reconnect Attempt to connect, line 219");
		// Attempt to connect
   // boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
   //                             1                  2                 3                      4                5                   6                        7
   //                           id         user        passwd                 willTopic qos retain Message
   // qos = 1
		if (PSclient.connect("LetterBox", "letterbox", "letterbox", (char *)TOPICBASE "State", 1, 0, "DEAD"))
		{
			Serial.println("reconnect successful, line 227");
			// Once connected, publish an announcement...
      Publish((char *)"State", (char *)"BOOTUP");
			// Subscribe to enable bi-directional comms.
			PSclient.subscribe(TOPICBASE "Config/#");  // Allow bootup config fetching using MQTT persist flag!
      PSclient.subscribe(TOPICBASE "Put/#");     // Send commands to this device, use Home/LetterBox/Get/# for responses.
		}
		else
			Serial.println("reconnect failed, line 235");
			delay(5000);
	} // end while (!PSclient.connected())
} // ----------------------------------------------------------------------------------end void reconnect()

// -----------------------------------------------------------------------------------void WiFiConnect()
void  WiFiConnect()
{
	delay(10);

  WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
 {
		Serial.println(".");
		delay(500);
 } // end while
 Serial.println("WiFi connected. Line 248 ");
} // ----------------------------------------------------------------------------------- end void WiFiConnect()


void  ICACHE_RAM_ATTR DeliverFunc()
{
	noInterrupts();
	
	if ((millis() - DeliverLastMillis) > confDeliverDelay)
	{
		DeliverPubFlag = 1;
		DeliverCount++;
   
   // -----------------------------
   /*
   if (!PSclient.connected()) 
   {
    Serial.println("WiFi client not connected");
   }

   Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
   String subject = "Mailbox delivery";
   if(gsender->Subject(subject)->Send("nestor@earthling.net", "Mail delivery")) {
        Serial.println("Message sent.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    } // end else
    */
   // ------------------------------
	}
	DeliverLastMillis = millis();
	
	interrupts();
}


void  ICACHE_RAM_ATTR CheckFunc()
{
	noInterrupts();
	
	if ((millis() - CheckLastMillis) > confCheckDelay)
	{
		CheckPubFlag = 1;
		DeliverCount = 0;
		CheckCount = 0;
	}
	// CheckCount++;
	CheckLastMillis = millis();
	
	interrupts();
}


void  ICACHE_RAM_ATTR callback(char* topic, byte* payload, unsigned int length)
{
	payload[length] = 0;    // Hack to be able to use this as a char string.

	if (strstr(topic, TOPICBASE "Config/"))
	{
		if (strstr(topic, "TempDelay"))
		    	confTempDelay = atoi((const char *)payload);
		
		else if (strstr(topic, "DeliverDelay"))
			confDeliverDelay = atoi((const char *)payload);
		
		else if (strstr(topic, "CheckDelay"))
			confCheckDelay = atoi((const char *)payload);

		else if (strstr(topic, "LuxDelay"))
			confLuxDelay = atoi((const char *)payload);
	}
}
