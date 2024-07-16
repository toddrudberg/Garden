
#include "DFRobot_SEN0385.h"

DFRobot_SHT3x sht3x(&Wire, 0x44, 4); // I2C

void cSEN0385::run385(sSoilSensorData* sensorData, time_t myTime)
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
                    if( sht3x.begin() != 0)
                    {
                        startState++;
                        timeOut = millis();
                        Serial.println("SEN0385 failed to start."); 
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
            static float tempSum = 0; // Sum of temperatures
            static int tempCount = 0; // Count of temperature readings
        
            struct tm *myTimeStruct = localtime(&myTime);
            int currentHour = myTimeStruct->tm_hour;

            if(millis() - lastRead > 1000)
            {
                lastRead = millis();
                sht3xData.temperature = (float)sht3x.getTemperatureF();
                sht3xData.humidity = (float)sht3x.getHumidityRH();
        
                unsigned long epochTime = sensorData->epochTime;
        
                // Check if current time is between 1400 (2 PM) and 1700 (5 PM)
                if(currentHour >= 14 && currentHour < 17)
                {
                    tempSum += sht3xData.temperature;
                    tempCount++;
                    sht3xData.avgOATPreviousDay = tempSum / tempCount;
                }
                else if(currentHour >= 17 && tempCount > 0) // Past 1700 and we have readings
                {
                    sht3xData.avgOATPreviousDay = tempSum / tempCount;
                    // Reset for the next day
                    tempSum = 0;
                    tempCount = 0;
                    // Optionally, do something with averageTemp, like storing or displaying it
                }

            }
            sensorData->outsideAirTemp = sht3xData.temperature;
            sensorData->outsideAirHumidity = sht3xData.humidity;
            sensorData->baroPressure = 0;
            sensorData->avgOATPreviousDay = sht3xData.avgOATPreviousDay;
            break;        
          }
        default:
            processState = 0;
            break;
    }
}





