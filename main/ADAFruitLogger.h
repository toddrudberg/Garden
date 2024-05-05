// cADAFRuitLogger.h
#ifndef ADAFUITLOGGER_H
#define SOIADAFUITLOGGER_HLSENSOR_H
#include "IO.h"
#include <max6675.h>
#include <SPI.h>
#include <SD.h>

class cAdafruitLogger
{
public:
  bool setupLogger();
  bool writeData(sSoilSensorData* soilSensorData);
    void RunLogger(sSoilSensorData* soilSensorData);
private:

};


#endif// ADAFUITLOGGER_H
