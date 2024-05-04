
#ifndef IO_H// IO.h
#define IO_H

#include <SoftwareSerial.h>

extern SoftwareSerial mySerial;
extern const int rs485Rx;
extern const int rs485Tx;
extern const int rs485TxEnable;
extern unsigned long baud;

#endif// IO.h