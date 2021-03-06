#ifndef ESPJsonProcession_h
#define ESPJsonProcession_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESPJsonProcession.h"


String digitalInputToJson(uint8_t pin, uint8_t value){
	//String kq = 
	return ("{\"digital_pin\":"+String(pin)+",\"digital_value\":"+String(value)+"}");//kq;
}

String analogInputToJson(uint8_t pin, int value){
	return ("{\"analog_pin\":"+String(pin)+",\"analog_value\":"+String(value)+"}");
}

String commandToJson(String command){
	return ("{\"command\":\""+command+"\"}");
}

String allToJson(uint8_t digital_pin, uint8_t digital_value, uint8_t analog_pin, int analog_value, String command){
	return ("{\"digital_pin\":"+String(digital_pin)+",\"digital_value\":"+String(digital_value)+",\"analog_pin\":"+String(analog_pin)+",\"analog_value\":"+String(analog_value)+",\"command\":\""+command+"\"}");
}

String handleJson(String Json){
	StaticJsonBuffer<200> esp_json;
	JsonObject& after_parse = esp_json.parseObject(Json);

	String temp_command = after_parse["command"];
	uint8_t temp_digital_pin = after_parse["digital_pin"];
	uint8_t temp_digital_value = after_parse["digital_value"];
	uint8_t temp_analog_pin = after_parse["analog_pin"];
	uint16_t temp_analog_value = after_parse["analog_value"];
	
	if(!((temp_digital_pin == 0) || (temp_digital_pin == 2) || (temp_digital_pin == 5) || ((temp_digital_pin >= 6) && (temp_digital_pin <= 11))))
		if(Json.indexOf("digital_pin") != -1)
			digitalWrite(temp_digital_pin, temp_digital_value = (temp_digital_value>0)? 1:0);

	if(!((temp_analog_pin == 0) || (temp_analog_pin == 2) || (temp_analog_pin == 5) || ((temp_analog_pin >= 6) && (temp_analog_pin <= 11))))
		if(Json.indexOf("analog_pin") != -1)
			analogWrite(temp_analog_pin, temp_analog_value);

	if(!((temp_digital_pin == 0) || (temp_digital_pin == 2) || (temp_digital_pin == 5) || ((temp_digital_pin >= 6) && (temp_digital_pin <= 11))))
	{	
		if(temp_command.compareTo("digital-read") == 0)
		{
			Json.replace("\"command\": \"digital-read\"", String("\"digital_value\": ")+String(digitalRead(temp_digital_pin)));
		}
	}

	if(temp_analog_pin >= 17)
	{
		if(temp_command.compareTo("analog-read") == 0)
		{
			Json.replace("\"command\": \"analog-read\"", String("\"analog_value\": ")+String(analogRead(temp_analog_pin)));
		}
	}

	return Json;
}

#endif