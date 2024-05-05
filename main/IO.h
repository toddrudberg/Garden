
#ifndef IO_H// IO.h
#define IO_H

#include <SoftwareSerial.h>

extern SoftwareSerial mySerial;
extern const int rs485Rx;
extern const int rs485Tx;
extern const int rs485TxEnable;
extern const unsigned int sdChipSelect;


extern const unsigned long baud;

struct sSoilSensorData
{
  char* timeStamp;
  double outsideAirTemp;
  double outsideAirHumidity;
  double soilTemperature;
  double soilElectricalConductivity;
  double soilMoisture;
  double soilPh;
}; 



extern const char* FileName;
extern char gTimeString[10];

#endif// IO.h