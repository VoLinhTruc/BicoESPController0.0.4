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

#define ONE_BYTE 1

#define NUM_OF_NETWORK_PARAMETER 5
#define NUM_OF_PART_IN_EACH_NETWORK_PARAMETER 4 //"part" in this case is octec

#define SOFTAP_LOCAL_ADDRESS 0
#define SOFTAP_GATEWAY 1
#define STATION_LOCAL_ADDRESS 2
#define STATION_GATEWAY 3
#define SUBNET 4

#define MAX_SSID_PASS_LENGTH 32
#define PARAMETER_EEPROM_POSITION 0
#define ENABLE_AP_PARAMETER_EEPROM_POSITION (NUM_OF_NETWORK_PARAMETER * NUM_OF_PART_IN_EACH_NETWORK_PARAMETER)
#define ENABLE_STATION_PARAMETER_EEPROM_POSITION (ENABLE_AP_PARAMETER_EEPROM_POSITION + ONE_BYTE)
#define AP_HIDDEN_PARAMETER_EEPROM_POSITION (ENABLE_STATION_PARAMETER_EEPROM_POSITION + ONE_BYTE)
#define ENABLE_STATIC_IP (AP_HIDDEN_PARAMETER_EEPROM_POSITION + ONE_BYTE)
#define SOFTAP_SSID_EEPROM_POSITION 50 //=50+32*0 = 50+MAX_SSID_PASS_LENGTH*0 //don't use "#define SOFTAP_SSID_EEPROM_POSITION 50+MAX_SSID_PASS_LENGTH*0" to make CPU faster 
#define SOFTAP_PASS_EEPROM_POSITION (SOFTAP_SSID_EEPROM_POSITION + MAX_SSID_PASS_LENGTH) //=50+32*1 = 50+MAX_SSID_PASS_LENGTH*1
#define WIFI_SSID_EEPROM_POSITION (SOFTAP_PASS_EEPROM_POSITION + MAX_SSID_PASS_LENGTH) //=50+32*2 = 50+MAX_SSID_PASS_LENGTH*2
#define WIFI_PASS_EEPROM_POSITION (WIFI_SSID_EEPROM_POSITION + MAX_SSID_PASS_LENGTH) //=50+32*3 = 50+MAX_SSID_PASS_LENGTH*3

#define ON 1
#define OFF 0

#include <Arduino.h> //co hay khong thi cung khong anh huong toi chuong trinh
#include <ESP8266WiFi.h>
#include <EEPROM.h>

void goSetup();

void turnOnSoftAP();
void turnOffSoftAP();
void turnOnStation();
void turnOffStation();

bool stringToOctec(String string, uint8_t& octec0, uint8_t& octec1, uint8_t& octec2, uint8_t& octec3);
void cleanAndWaitingSerial();

IPAddress network_parameter[NUM_OF_NETWORK_PARAMETER];
//network_parameter[0] is softAP_localIP
//network_parameter[1] is STA_localIP
//network_parameter[2] is station's gateway
//network_parameter[3] is AP gateway
//network_parameter[4] is subnet

char ESP_ssid[MAX_SSID_PASS_LENGTH];
char ESP_password[MAX_SSID_PASS_LENGTH];
char SSID_AP[MAX_SSID_PASS_LENGTH];
char PASS_AP[MAX_SSID_PASS_LENGTH];

char softAP_turn;

//-------------------------------------------------------------------------------------------------------------
void doAfterTurnOnESP(int waiting_time)
{
	//Get IPv4 and configure data in EEPROM-----------------------------------
	EEPROM.begin(200);
	{
		for(int i = 0; i<NUM_OF_NETWORK_PARAMETER; i++)
		{  
			for(int j = 0; j<NUM_OF_PART_IN_EACH_NETWORK_PARAMETER; j++)
			{
				network_parameter[i].setAddressByte(j, EEPROM.read(PARAMETER_EEPROM_POSITION+(NUM_OF_PART_IN_EACH_NETWORK_PARAMETER*i+j)));
			}
		}

		for(int i = 0; i<MAX_SSID_PASS_LENGTH; i++)
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
		if(EEPROM.read(ENABLE_AP_PARAMETER_EEPROM_POSITION)==49)
		{
			turnOnSoftAP();
		}
		else
		{
			turnOffSoftAP();
		}

		//---------------------------------
		if(EEPROM.read(ENABLE_STATION_PARAMETER_EEPROM_POSITION)==49)
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
		uint8_t choose = 1; //init the true value before goto while loop
		while(choose)
		{
			Serial.println();
			Serial.println("1. IPv4");
			Serial.println("2. AP configurations");
			Serial.println("3. Station configurations");
			Serial.println("4. Allow to turn wifi on");
			Serial.println("5. Enable Static IP");
			Serial.println("0. Exist");

			cleanAndWaitingSerial();
			choose = uint8_t(Serial.readString().toInt());


			switch(choose)
			{
				case 1:
				{
					//Change IPv4 parameters-------------------------------------------------
					{
						for(uint8_t i = 0; i < NUM_OF_NETWORK_PARAMETER; i++)
						{
							switch (i)
							{
								case 0:{Serial.print("Soft_AP Local IP: "); break;}
								case 1:{Serial.print("AP Gateway: "); break;}
								case 2:{Serial.print("Station Local IP: "); break;}
								case 3:{Serial.print("Station Gateway: "); break;}
								case 4:{Serial.print("Subnet: "); break;}
								default: break;
							}

							Serial.print("(Pre parameter: ");
							for(uint8_t j = 0; j<NUM_OF_PART_IN_EACH_NETWORK_PARAMETER; j++)
							{
								Serial.print(network_parameter[i].getAddressByte(j));
								//if j is not the last octec index
								if(j != NUM_OF_PART_IN_EACH_NETWORK_PARAMETER-1)
								{
									Serial.print('.');
								}
							}
							Serial.println(")");

							bool correctly = 0;
							while(!correctly)
							{
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									uint8_t oct[NUM_OF_PART_IN_EACH_NETWORK_PARAMETER]; //NUM_OF_PART_IN_EACH_NETWORK_PARAMETER = 4 octec 
									if(stringToOctec(temp_string, oct[0], oct[1], oct[2], oct[3]))
									{
										for(int j = 0; j<NUM_OF_PART_IN_EACH_NETWORK_PARAMETER; j++)
										{
											EEPROM.write(NUM_OF_PART_IN_EACH_NETWORK_PARAMETER*i+j, oct[j]);
										}
										correctly = 1;
										Serial.println(temp_string);
										Serial.println();
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
						Serial.print("SSID_AP: ");
						Serial.print("(Pre SSID_AP: ");
						Serial.print(SSID_AP);
						Serial.println(")");

						cleanAndWaitingSerial();
						temp_string = Serial.readString();
						if(temp_string != "`")
						{
							temp_string.toCharArray(SSID_AP, temp_string.length()+1);
							Serial.println(SSID_AP);
						}

						Serial.print("PASS_AP:");
						Serial.print("(Pre PASS_AP: ");
						Serial.print(PASS_AP);
						Serial.println(")");

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
						//48, 49 are the ASCII code of '0' and '1'
						if(temp_val == 49)
							EEPROM.write(AP_HIDDEN_PARAMETER_EEPROM_POSITION, 49);
						else if(temp_val == 48)
							EEPROM.write(AP_HIDDEN_PARAMETER_EEPROM_POSITION, 48);
						
						for(int i = 0; i<MAX_SSID_PASS_LENGTH; i++)
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
						//48, 49 are the ASCII code of '0' and '1'
						if(temp_val != '`')
						{
							if(temp_val == 49)
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
								uint8_t number_of_wifi_to_scan = 5;
								for (int i = 0; i < number_of_wifi_to_scan; i++)
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
								

								for(int i = 0; i<MAX_SSID_PASS_LENGTH; i++)
								{
									EEPROM.write(i+WIFI_SSID_EEPROM_POSITION, ESP_ssid[i]);
									EEPROM.write(i+WIFI_PASS_EEPROM_POSITION, ESP_password[i]);
								}
								EEPROM.commit();
							}
							else
							{
								Serial.print("SSID:");
								Serial.print("(Pre SSID: ");
								Serial.print(ESP_ssid);
								Serial.println(")");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(ESP_ssid, temp_string.length()+1);
									Serial.println(temp_string);
								}
								Serial.print("PASSWORD:");
								Serial.print("(Pre PASSWORD: ");
								Serial.print(ESP_password);
								Serial.println(")");
								cleanAndWaitingSerial();
								temp_string = Serial.readString();
								if(temp_string != "`")
								{
									temp_string.toCharArray(ESP_password, temp_string.length()+1);
									Serial.println(temp_string);
								}
								for(int i = 0; i<MAX_SSID_PASS_LENGTH; i++)
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
						Serial.print("Turn on AP: (1:Y; 0:N)");
						Serial.print("(Pre parameter: ");
						Serial.print(EEPROM.read(ENABLE_AP_PARAMETER_EEPROM_POSITION));
						Serial.println(")");

						cleanAndWaitingSerial();
						uint8_t temp_val = Serial.read();
						if(temp_val != '`')
						{
							EEPROM.write(ENABLE_AP_PARAMETER_EEPROM_POSITION, temp_val);
							EEPROM.commit();
							Serial.println(temp_val);
							Serial.println();
						}

						Serial.print("Turn on Sta: (1:Y; 0:N)");
						Serial.print("(Pre parameter: ");
						Serial.print(EEPROM.read(ENABLE_STATION_PARAMETER_EEPROM_POSITION));
						Serial.println(")");

						cleanAndWaitingSerial();
						temp_val = Serial.read();
						if(temp_val != '`')
						{
							EEPROM.write(ENABLE_STATION_PARAMETER_EEPROM_POSITION, temp_val);
							EEPROM.commit();
							Serial.println(temp_val);
							Serial.println();
						}
					}
					break;
				}

			 	case 5:
			 	{
				 	//Allow to turn Wifi on or not------------------------------------------
				 	{
						Serial.print("Enable Static IP: (1:Y; 0:N)");
						Serial.print("(Pre parameter: ");
						Serial.print(EEPROM.read(ENABLE_STATIC_IP));
						Serial.println(")");

						cleanAndWaitingSerial();
						uint8_t temp_val = Serial.read();
						if(temp_val != '`')
						{
							EEPROM.write(ENABLE_STATIC_IP, temp_val);
							EEPROM.commit();
							Serial.println(temp_val);
							Serial.println();
						}
					}
					break;
				}
				default: break;
			}
		}

		//get data from epprom to network variable to config network
		for(int i = 0; i<NUM_OF_NETWORK_PARAMETER; i++)
		{  
			for(int j = 0; j<NUM_OF_PART_IN_EACH_NETWORK_PARAMETER; j++)
			{
				network_parameter[i].setAddressByte(j, EEPROM.read(PARAMETER_EEPROM_POSITION+(NUM_OF_PART_IN_EACH_NETWORK_PARAMETER*i+j)));
			}
		}	
		Serial.println();
	}
	delay(100);
	EEPROM.end();
}

//---------------------------------------------------------------------------------------------------------------
void turnOnSoftAP()
{
	Serial.print("Conf AP:");
	Serial.println(WiFi.softAPConfig(network_parameter[SOFTAP_LOCAL_ADDRESS], network_parameter[SOFTAP_GATEWAY], network_parameter[SUBNET])? "OK":"Failed");
	Serial.print("Init AP:");
	//48, 49 are the ASCII code of '0' and '1'
	//7 is the wifi channel (RF channel)
	if(EEPROM.read(AP_HIDDEN_PARAMETER_EEPROM_POSITION) == 49)
	{
		Serial.println(WiFi.softAP(SSID_AP, PASS_AP, 7, 1)? "OK":"Failed");
	}
	else
	{
		Serial.println(WiFi.softAP(SSID_AP, PASS_AP, 7, 0)? "OK":"Failed");
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
	Serial.println(WiFi.config(network_parameter[STATION_LOCAL_ADDRESS], network_parameter[STATION_GATEWAY], network_parameter[SUBNET])? "OK":"Failed");
	// WiFi.disconnect(true);
	// delay(1000);
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
	WiFi.disconnect();
}

//-------------------------------------------------------------------------------------------------------------
bool stringToOctec(String string, uint8_t& octec0, uint8_t& octec1, uint8_t& octec2, uint8_t& octec3)
{
  String oct[NUM_OF_PART_IN_EACH_NETWORK_PARAMETER]; //NUM_OF_PART_IN_EACH_NETWORK_PARAMETER = 4 octec
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