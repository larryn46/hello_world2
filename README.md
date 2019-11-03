# hello_world2 edited 11/03/2019 0321 pst<br>
use 17j ~/Arduino/credentials.h for ssid and password<br>
use RP3 model B running dietpi96 for MQTT broker at 192.168.1.96 see https://dietpi.com/<br>
use 17j ~/Arduino/ESP8266MQTTletterbox192.168.1.96 for MQTT publisher. This displays<br>

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
$ mosquitto_sub -h 192.168.1.96 -t Home/LetterBox/+ -v<br>
to display MQTT messages plus date and time, in linux bash, try:<br>
mosquitto_sub -h 192.168.1.96 -t Home/LetterBox/+ -v | while read line ; do  echo -e  "$(date)\t $line" ; done<br>
updated 11/03/2019 0321 PST

