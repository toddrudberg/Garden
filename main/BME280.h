#ifndef BME280_H
#define BME280_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "IO.h"

struct sBME280Data
{
  float temperature;
  float humidity;
  float pressure;
};

class cBME280 {
public:
    bool startBME();
    sBME280Data readBME();
    void runBME(sSoilSensorData* sensorData);

};

#endif // BME280_H