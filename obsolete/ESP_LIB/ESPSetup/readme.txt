/*
->At first, must add this two fucntion to IPAddress.cpp to run ESPSetup.h (this file) correctly

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


->And add this two prototype to IPAddress.h

uint8_t setAddressByte(uint8_t which_byte, uint8_t value);
uint8_t getAddressByte(uint8_t which_byte);
*/