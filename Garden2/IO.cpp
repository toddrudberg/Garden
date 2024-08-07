#include "IO.h"


const int rs485rX = 0;  //Definition RS485 shield RX terminal
const int rs485tX = 1;  //Definition RS485 shield TX terminal
const int rs485TxEnable = 2;  //Definition RS485 shield enable terminal 

//rtc pins 4,5
const unsigned int sdChipSelect = 10; // SD card chip select
//11 SPI MOSI
//12 SPI MISO
//13 SPI SCK

const unsigned int Valve1 = 6;
const unsigned int Valve2 = 7;
const unsigned int Valve3 = 8;

const unsigned long baud = 9600;
SoftwareSerial mySerial(rs485rX, rs485tX); // RX, TX
cAdafruitLogger logger;

const char* FileName = "Log.csv";

char gTimeString[10] = "00:00:00";

unsigned long gWateringTimeStart = 0;
unsigned long gWateringDuration = 0;
bool gWatering = false;