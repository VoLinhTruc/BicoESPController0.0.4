#include <ESPJsonProcession.h>

uint8_t state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(3, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  state = !state;
  String string1 = digitalInputToJson(3, state);
  Serial.println(string1);
  Serial.println(analogInputToJson(15, analogRead(15)));
  Serial.println(commandToJson("abcxyz"));
  String string2 = allToJson(3,state, 5,random(255), "abcxxyz");
  Serial.println(string2);

  handleJson(string2);

  delay(1000);
}
