#include <SoftwareSerial.h>

#define STAYIN(m,n) ((temp_char >= m) && (temp_char <= n))

SoftwareSerial ESP(2, 4);

String message = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ESP.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(ESP.available() > 0)
  {
    String temp_string = "";
    while(ESP.available() > 0)
    {
      delayMicroseconds(300); 
      temp_string += (char)ESP.read();
    }
    message += temp_string;
    while(message.length() > 150)
    {
      message = message.substring(message.indexOf('\n')); 
      message = message.substring(1);
    } 
    Serial.print("display.txt=\"" + message + "\"" + (char)0xff + (char)0xff + (char)0xff);
  }

  if(Serial.available() > 0)
  {
    String temp_string = "";
    while(Serial.available() > 0)
    {
      delayMicroseconds(100);
      temp_string += (char)Serial.read();
    }
    delay(200);
    ESP.print(temp_string);
  }
}
