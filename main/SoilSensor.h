// cSoilSensor.h
#ifndef SOILSENSOR_H
#define SOILSENSOR_H
#include "IO.h"

class cSoilSensor
{
public:
  void setupSoilSensor();
  bool checkSoilSensor(sSoilSensorData* soilSensorData); //returns true if data is new
  void runSoilSensor(sSoilSensorData* soilSensorData);

private:
    const uint8_t instructionToSoilSensor[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09 }; //Command to read data
    const int msBetweenReads = 1000;
    sSoilSensorData fillSoilSensorDataArray(sSoilSensorData* soilSensorDataArray, sSoilSensorData newSoilSensorData);
};


#endif// SOILSENSOR_H
