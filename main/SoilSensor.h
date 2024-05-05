// cSoilSensor.h
#ifndef SOILSENSOR_H
#define SOILSENSOR_H
#include "IO.h"

class cSoilSensor
{
public:
  void setupSoilSensor();
  void checkSoilSensor(sSoilSensorData* soilSensorData);
  void runSoilSensor(sSoilSensorData* soilSensorData);

private:
    private:
    const uint8_t instructionToSoilSensor[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09 }; //Command to read data
    const int msBetweenReads = 1000;
};


#endif// SOILSENSOR_H
