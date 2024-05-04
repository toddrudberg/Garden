#include "IO.h"

const int RS485_RX = 0;  //Definition RS485 shield RX terminal
const int RS485_TX = 1;  //Definition RS485 shield TX terminal
const int RS485_TX_Enable = 2;  //Definition RS485 shield enable terminal 
unsigned long baud = 9600;
SoftwareSerial mySerial(RS485_RX, RS485_TX); // RX, TX