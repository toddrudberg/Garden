#include "IO.h"

const int rs485rX = 0;  //Definition RS485 shield RX terminal
const int rs485tX = 1;  //Definition RS485 shield TX terminal
const int rs485TxEnable = 2;  //Definition RS485 shield enable terminal 
unsigned long baud = 9600;
SoftwareSerial mySerial(rs485rX, rs485tX); // RX, TX