# hello_world2 edited 8/15/2018 1354 pdt<br>
use 17j ~/Arduino/credentials.h for ssid and password<br>
use 17j ~/Arduino/uMQTTBroker for MQTT broker at 192.168.1.94<br>
use 17j ~/Arduino/ESP8266MQTTletterbox for MQTT publisher. This displays<br>

Lux = 2.00<br>
Mailbox Temp = 22.00<br>
Mailbox humidity = 33.00<br>
Humidity Index = 21.12<br>

<br> 
on serial monitor and <br>
Home/LetterBox/Lux  2.00<br>
Home/LetterBox/Temperature 23.00<br>
Home/LetterBox/Humidity 31.00<br>
Home/LetterBox/HeatIndex 22.16<br>
on a remote box running <br>
$ mosquitto_sub -h 192.168.1.94 -t Home/LetterBox/+ -v<br>
updated 10/29/2019 0531 PDT

