#ifndef DFROBOT_SEN0385_H
#define DFROBOT_SEN0385_H

#include <DFRobot_SHT3x.h>

#include "IO.h"

struct sSEN0385Data
{
    float temperature;
    float humidity;
    float avgOATPreviousDay;
};

class cSEN0385 
{
    public:
        void run385(sSoilSensorData* sensorData, time_t epochTime);
};

#endif // DFROBOT_SEN0385_H