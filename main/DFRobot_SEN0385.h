#ifndef DFROBOT_SEN0385_H
#define DFROBOT_SEN0385_H

#include <DFRobot_SHT3x.h>

#include "IO.h"

struct sSEN0385Data
{
    float temperature = -1;
    float humidity = -1;
    float avgOATPreviousDay = -1;
};

class cSEN0385 
{
    public:
        void run385(sSoilSensorData* sensorData, time_t epochTime);
};

#endif // DFROBOT_SEN0385_H