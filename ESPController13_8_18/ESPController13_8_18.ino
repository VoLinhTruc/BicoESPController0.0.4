#include <ESP8266WiFi.h>
#include "ESPSetup/ESPSetup.h"
#include "ESPJsonProcession/ESPJsonProcession.h"

WiFiServer esp_server(80);

const uint8_t D[] = {16, 5, 4, 14, 12, 13}; // cac pin nay da duoc test hoat dong on dinh, cac chan I/O con lai khong nen dung

String http_header = 
            String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "User-Agent: BiCoESPController\r\n" +
            //"Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "\r\n" +         
            "\r\n";


void setup() {
  // put your setup code here, to run once:  

  for(uint8_t dem = 0; dem < sizeof(D)/sizeof(uint8_t); dem++)
  {
    pinMode(D[dem], OUTPUT);
    digitalWrite(D[dem], HIGH);
  }
  
  Serial.begin(9600);
  Serial.setTimeout(100);
  delay(1);

  doAfterTurnOnESP(5000);

  esp_server.begin();
  delay(1);
  Serial.printf("Web server started, open %s in a web brower \n\r", WiFi.localIP().toString().c_str());
}


void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient  client_obj = esp_server.available();
  if (client_obj) 
  {
    //Serial.println("\n**Client connected**");
    String document;
    delay(10);
    while(client_obj.available())
    {
      delayMicroseconds(200);  
      document += char(client_obj.read());
    }
    String received_json = handleJson(document.substring(document.indexOf("{"), document.indexOf("}") + 1));
    //feedback block
    {
      client_obj.println(http_header+received_json);
      delay(10);
    }
    Serial.println(document);
    //Serial.println(received_json);
    client_obj.stop();
    //Serial.println("**Client disconnect**");
  }
}
