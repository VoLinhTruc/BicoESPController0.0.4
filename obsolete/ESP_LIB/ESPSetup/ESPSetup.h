/*
	At first, must add this two fucntion to IPAddress.cpp to run ESPSetup.h (this file) correctly

uint8_t IPAddress::setAddressByte(uint8_t which_byte, uint8_t value){
if(which_byte < 4){
    _address.bytes[which_byte] = value;
    return value;
}
else return -1;
}

uint8_t IPAddress::getAddressByte(uint8_t which_byte)
{
    return _address.bytes[which_byte];
}

	and add this two prototype to IPAddress.h

uint8_t setAddressByte(uint8_t which_byte, uint8_t value);
uint8_t getAddressByte(uint8_t which_byte);
*/

#ifndef espSetup_h
#define espSetup_h

#define MAX_SSID_PASS_LENGTH 32
#define PARAMETER_EEPROM_POSITION 0
#define SOFTAP_SSID_EEPROM_POSITION 50 //=50+32*0 = 50+MAX_SSID_PASS_LENGTH*0
#define SOFTAP_PASS_EEPROM_POSITION 82 //=50+32*1 = 50+MAX_SSID_PASS_LENGTH*1
#define WIFI_SSID_EEPROM_POSITION 114 //=50+32*2 = 50+MAX_SSID_PASS_LENGTH*2
#define WIFI_PASS_EEPROM_POSITION 146 //=50+32*3 = 50+MAX_SSID_PASS_LENGTH*3

#define ON 1
#define OFF 0

//#include <Arduino.h> //co hay khong thi cung khong anh huong toi chuong trinh
#include <ESP8266WiFi.h>
#include <EEPROM.h>

void goSetup();

void turnOnSoftAP();
void turnOffSoftAP();
void turnOnStation();
void turnOffStation();

bool stringToOctec(String string, uint8_t& octec0, uint8_t& octec1, uint8_t& octec2, uint8_t& octec3);
void cleanAndWaitingSerial();

IPAddress network_address[4];
//network_address[0] is softAP_localIP
//network_address[1] is STA_localIP
//network_address[2] is subnet
//network_address[3] is gateway

char ESP_ssid[32];
char ESP_password[32];
char SSID_AP[32];
char PASS_AP[32];

char softAP_turn;

//-------------------------------------------------------------------------------------------------------------
void doAfterTurnOnESP(int waiting_time)
{
	//Get IPv4 and configure data in EEPROM-----------------------------------
	EEPROM.begin(200);
	{
		for(int i = 0; i<4; i++)
		{  
			for(int j = 0; j<4; j++)
			{
				network_address[i].setAddressByte(j, EEPROM.read(PARAMETER_EEPROM_POSITION+(4*i+j)));
			}
		}

		for(int i = 0; i<32; i++)
		{
			SSID_AP[i] = EEPROM.read(i+SOFTAP_SSID_EEPROM_POSITION);
			PASS_AP[i] = EEPROM.read(i+SOFTAP_PASS_EEPROM_POSITION);
			ESP_ssid[i] = EEPROM.read(i+WIFI_SSID_EEPROM_POSITION);
			ESP_password[i] = EEPROM.read(i+WIFI_PASS_EEPROM_POSITION);
		}	
	}
	EEPROM.end();
	Serial.println();
	Serial.println();

	//Waiting to setup-----------------------------------------
	{
		int startWaitToSetup = millis();
		Serial.println("Go Setup?");

		while(millis() - startWaitToSetup < waiting_time)
		{
			//Serial.print(".");
			delay(200);

			if(Serial.available() > 0)
			{
				Serial.println();
				goSetup();
				break;
			}
		}
		Serial.println();
	}

	//Get data from EEPROM, this data allow turn on wifi configurations or not-----
	EEPROM.begin(200);
	{
		if(EEPROM.read(16)==49)
		{
			turnOnSoftAP();
		}
		else
		{
			turnOffSoftAP();
		}

		//---------------------------------
		if(EEPROM.read(17)==49)
		{
			turnOnStation();
		}
		else
		{
			turnOffStation();
		}
	}
	EEPROM.end();
}


//---------------------------------------------------------------------------------------------------------------
void goSetup()
{ 
	String temp_string;
	EEPROM.begin(200);
	{
		//Change IPv4 and wifi configurations-----------------------------
		uint8_t choose = 4;
		while(choose)
		{
			Serial.println();
			Serial.println("1. IPv4");
			Serial.println("2. AP configurations");
			Serial.println("3. Station configurations");
			Serial.println("4. Allow to turn wifi on");
			Serial.println("0. Exist");

			cleanAndWaitingSerial();
			choose = uint8_t(Serial.readString().toInt());


			switch(choose)
			{
				case 1:
				{
					//Change IPv4 parameters-------------------------------------------------
					{
						for(uint8_t i = 0; i < 4; i++)
						{
							switch (i)
							{
								case 0:{Serial.println("Soft_APIP:"); break;}
								case 1:{Serial.println("Local_IP:"); break;}
								case 2:{Serial.println("Gateway:"); break;}
								case 3:{Serial.println("Subnet:"); break;}
								default: break;
							}

							bool correctly = 0;
							while(!correctly)
							{
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									uint8_t oct[4];
									if(stringToOctec(temp_string, oct[0], oct[1], oct[2], oct[3]))
									{
										for(int j = 0; j<4; j++)
										{
											EEPROM.write(4*i+j, oct[j]);
										}
										correctly = 1;
										Serial.println(temp_string);
									}
									else
									{
										Serial.println("Invaild input");
									}
								}
								else break;
							}
						}
						EEPROM.commit();
					}
					break;
				}

				case 2:
				{
					//Change soft-AP configurations----------------------------------------
					{ 
								Serial.println("SSID_AP:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(SSID_AP, temp_string.length()+1);
									Serial.println(SSID_AP);
								}

								Serial.println("PASS_AP:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(PASS_AP, temp_string.length()+1);
									Serial.println(PASS_AP);
								} 


								Serial.println("AP hidden?: (1:Yes; 0:No)");
								cleanAndWaitingSerial();
								uint8_t temp_val = Serial.read();
								if(temp_val == '`')
								{
									if(temp_val == 49)
										EEPROM.write(18, 49);
									else if(temp_val == 48)
										EEPROM.write(18, 48);
								}
								for(int i = 0; i<32; i++)
								{
									EEPROM.write(i+SOFTAP_SSID_EEPROM_POSITION, SSID_AP[i]);
									EEPROM.write(i+SOFTAP_PASS_EEPROM_POSITION, PASS_AP[i]);
								}
								EEPROM.commit();
					}
					break;
				}

				case 3:
				{
					//Change station configurations----------------------------------------------------
					{
						Serial.println("Inputing or scanning ? (0:Inp; 1:Scan)");
						cleanAndWaitingSerial();
						uint8_t temp_val = Serial.read();
						if(temp_val != '`')
						{
							if(temp_val-48 == 1)
							{
								WiFi.disconnect();
								delay(100);

								Serial.println("Scanning..");    
								WiFi.scanNetworks(true, true);
								int numOfAP = -1;
								while(numOfAP < 0)
								{
									numOfAP = WiFi.scanComplete();
									//Serial.print("."); 
									delay(500);
								}

								Serial.println();
								for (int i = 0; i < 5; i++)
								{
									//Serial.printf("%d: %s, Ch:%d %s\n\r", i, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
									Serial.print(i);
									Serial.print(": ");
									Serial.print(WiFi.SSID(i));
									Serial.print(", ");
									Serial.print(WiFi.channel(i));
									Serial.print(", ");
									Serial.print(WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
									Serial.println();
								}

								Serial.println("Choose WiFi:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									int wifiSel = 0;
									wifiSel = uint8_t(temp_string.toInt());
									WiFi.SSID(wifiSel).toCharArray(ESP_ssid, WiFi.SSID(wifiSel).length()+1);
									Serial.println(ESP_ssid);
								}
								Serial.println("Password:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(ESP_password, temp_string.length()+1); 
									Serial.println(temp_string);
								}
								

								for(int i = 0; i<32; i++)
								{
									EEPROM.write(i+WIFI_SSID_EEPROM_POSITION, ESP_ssid[i]);
									EEPROM.write(i+WIFI_PASS_EEPROM_POSITION, ESP_password[i]);
								}
								EEPROM.commit();
							}
							else
							{
								Serial.println("SSID:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(ESP_ssid, temp_string.length()+1);
									Serial.println(temp_string);
								}
								Serial.println("PASSWORD:");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(ESP_password, temp_string.length()+1);
									Serial.println(temp_string);
								}
								for(int i = 0; i<32; i++)
								{
									EEPROM.write(i+WIFI_SSID_EEPROM_POSITION, ESP_ssid[i]);
									EEPROM.write(i+WIFI_PASS_EEPROM_POSITION, ESP_password[i]);
								}
								EEPROM.commit();
							}
						}
					}
					break;
				}

			 	case 4:
			 	{
				 	//Allow to turn Wifi on or not------------------------------------------
				 	{
						Serial.println("Turn on AP: (1:Y; 0:N)");
						cleanAndWaitingSerial();
						uint8_t temp_val = Serial.read();
						if(temp_val != '`')
						{
							EEPROM.write(16, temp_val);
							EEPROM.commit();
							Serial.println(EEPROM.read(16));
						}
						Serial.println("Turn on Sta: (1:Y; 0:N)");
						cleanAndWaitingSerial();
						temp_val = Serial.read();
						if(temp_val != '`')
						{
							EEPROM.write(17, temp_val);
							EEPROM.commit();
							Serial.println(EEPROM.read(17));
						}
					}
					break;
				}

				default: break;
			}
		}

		for(int i = 0; i<4; i++)
		{  
			for(int j = 0; j<4; j++)
			{
				network_address[i].setAddressByte(j, EEPROM.read(PARAMETER_EEPROM_POSITION+(4*i+j)));
			}
		}	
		Serial.println();
	}
	EEPROM.end();
}

//---------------------------------------------------------------------------------------------------------------
void turnOnSoftAP()
{
	Serial.print("Conf AP:");
	Serial.println(WiFi.softAPConfig(network_address[0], network_address[2], network_address[3])? "OK":"Failed");
	Serial.print("Init AP:");
	if(EEPROM.read(18) == 49)
	{
		Serial.println(WiFi.softAP(SSID_AP, PASS_AP, 1, 1)? "OK":"Failed");
	}
	else
	{
		Serial.println(WiFi.softAP(SSID_AP, PASS_AP, 1, 0)? "OK":"Failed");
	}
	Serial.print("AP addr:");
	Serial.println(WiFi.softAPIP());

	softAP_turn = ON;
}

void turnOffSoftAP()
{
	WiFi.softAPdisconnect(true);
	
	softAP_turn = OFF;
}


void turnOnStation()
{
	Serial.print("Conf STA:");
	Serial.println(WiFi.config(network_address[1], network_address[2], network_address[3])? "OK":"Failed");
	WiFi.disconnect(true);
	delay(1000);
	Serial.println("Connect to " + String(ESP_ssid));
	WiFi.begin(ESP_ssid, ESP_password);
	long connecting_time = millis();
	uint8_t check_failed = 0;
	while(WiFi.status() != WL_CONNECTED)
	{
		//Serial.print(".");
		delay(500);
		if(check_failed == 0)
		{
			if((millis() - connecting_time) > 10000)
			{
				Serial.println("Connecting Failed!");
				check_failed = 1;
			}
		}
	}
	Serial.println();
	Serial.print("FIX IP:");
	Serial.println(WiFi.localIP());
}
void turnOffStation()
{
	WiFi.disconnect(true);
}

//-------------------------------------------------------------------------------------------------------------
bool stringToOctec(String string, uint8_t& octec0, uint8_t& octec1, uint8_t& octec2, uint8_t& octec3)
{
  String oct[4];
  uint8_t oct_order = 0;
  uint8_t string_length = string.length();

  if(string_length < 11)
    return 0;
  
  for (int i = 0; i < string_length; i++)
  {
    if(string[i] != '.')
      oct[oct_order]+=string[i];
    else 
      oct_order++;
  }

  if(oct_order > 3)
    return 0;
  else
  {
    octec0 = uint8_t(oct[0].toInt());
    octec1 = uint8_t(oct[1].toInt());
    octec2 = uint8_t(oct[2].toInt());
    octec3 = uint8_t(oct[3].toInt());

    return 1;
  }
}

void cleanAndWaitingSerial()
{
  while(Serial.available())
    Serial.read();
  while(!Serial.available()){
    //Serial.print(".");
    delay(500);
  }
  Serial.println();
}
//-------------------------------------------------------------------------------------------------------------
#endif