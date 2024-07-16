#ifndef DFROBOT_SEN0385_H
#define DFROBOT_SEN0385_H

#include <DFRobot_SHT3x.h>

#include "IO.h"

struct sSEN0385Data
{
    float temperature;
    float humidity;
};

class cSEN0385 
{
    public:
        void run385(sSoilSensorData* sensorData);
};

#endif // DFROBOT_SEN0385_H