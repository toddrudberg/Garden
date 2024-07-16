
#include "DFRobot_SEN0385.h"

DFRobot_SHT3x sht3x(&Wire, 0x44, -1); // I2C

void cSEN0385::run385(sSoilSensorData* sensorData)
{
    static int processState = 0;

    switch (processState)
    {
        case 0:
        {
            static int startState = 0;
            static unsigned long timeOut = millis();
            switch(startState)
            {
                case 0:
                    if(!sht3x.begin())
                    {
                        startState++;
                        timeOut = millis();
                    }
                    else 
                    {
                        Serial.println("SEN0385 started successfully.");
                        processState++;
                    }
                    break;
                case 1:
                    if(millis() - timeOut > 1000)
                    {
                        startState = 0;
                    }
                    break;
            }
            break;
        }
        case 1:
        {
            static int lastRead = millis();
            static sSEN0385Data sht3xData;
            if(millis() - lastRead > 1000)
            {

                lastRead = millis();
                sht3xData.temperature = (float)sht3x.getTemperatureF();
                sht3xData.humidity = (float)sht3x.getHumidityRH();
            }
            sensorData->outsideAirTemp = sht3xData.temperature;
            sensorData->outsideAirHumidity = sht3xData.humidity;
            sensorData->baroPressure = 0;
            break;
        }
        default:
            processState = 0;
            break;
    }
}





