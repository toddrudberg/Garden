
#ifndef ADAFUITLOGGER_H
#define ADAFUITLOGGER_H
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

    void RunLogger(sSoilSensorData* soilSensorData, bool wifiConnectionFailed);

    bool setupRTC();
    char* getExcelFormattedDate();
    char* getExcelFormattedTime();
    uint32_t getUnixTime();
    void SetTime(unsigned long unixTime);
private:
    bool writeData(sSoilSensorData* soilSensorData, bool wifiConnectionFailed);
};
#endif// ADAFUITLOGGER_H
