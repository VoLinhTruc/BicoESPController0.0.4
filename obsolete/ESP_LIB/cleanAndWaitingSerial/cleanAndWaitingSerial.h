#ifndef cleanAndWaitingSerial_h
#define cleanAndWaitingSerial_h

#include <Arduino.h>

void cleanAndWaitingSerial(){
  while(Serial.available())
    Serial.read();
  while(!Serial.available()){
    //Serial.print(".");
    delay(500);
  }
  Serial.println();
}

#endif