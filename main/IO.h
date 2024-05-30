
#ifndef IO_H// IO.h
#define IO_H
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <limits.h>
#include <EEPROM.h>

extern SoftwareSerial mySerial;

extern const char remoteServer[];
extern const int remoteServerPort;

extern const int rs485Rx;
extern const int rs485Tx;
extern const int rs485TxEnable;
extern const unsigned int sdChipSelect;
extern const unsigned int Valve1;
extern const unsigned int Valve2;
extern const unsigned int Valve3;

extern const unsigned long baud;

extern bool rtcFailed;

struct sSoilSensorData
{
  char* dateStamp;
  char* timeStamp;
  unsigned long epochTime;
  float outsideAirTemp;
  float outsideAirHumidity;
  float baroPressure;
  float soilTemperature;
  float soilElectricalConductivity;
  float soilMoisture;
  float soilPh;
}; 

struct sTotalState
{
  sSoilSensorData soilSensorData;
  unsigned long wateringTimeStart;
  unsigned long wateringDuration;
  bool watering;
  bool autoWaterCycleActive;
};

#include "ADAFruitLogger.h"
extern cAdafruitLogger logger;

extern const char* FileName;
// extern char gTimeString[10];
extern unsigned long gWateringTimeStart;
extern unsigned long gWateringDuration;
extern bool gManualWateringOn;
extern bool gAutoWateringEnabled;
extern bool gAutoWateringCycleOn;
extern unsigned long gremoteServerFails;
#endif// IO.h