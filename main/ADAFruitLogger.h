// cADAFRuitLogger.h
#ifndef ADAFUITLOGGER_H
#define SOIADAFUITLOGGER_HLSENSOR_H
#include "IO.h"
#include <max6675.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

extern RTC_PCF8523 rtc;

extern char daysOfTheWeek[7][12];


class cAdafruitLogger
{
public:
    bool setupLogger();
    bool writeData(sSoilSensorData* soilSensorData);
    void RunLogger(sSoilSensorData* soilSensorData);

    bool setupRTC();
    char* getExcelFormattedDate();
    char* getExcelFormattedTime();
private:

};


#endif// ADAFUITLOGGER_H
