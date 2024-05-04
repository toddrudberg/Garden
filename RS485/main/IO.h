// IO.h
#include <SoftwareSerial.h>

extern SoftwareSerial mySerial;
extern const int RS485_RX;
extern const int RS485_TX;
extern const int RS485_TX_Enable;
extern unsigned long baud;

void SetupSoilSensor();
void CheckSoilSensor();