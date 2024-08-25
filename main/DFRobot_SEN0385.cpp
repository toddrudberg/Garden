
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
            static bool avgCalculatedForDay = false; // Flag to track if average is calculated for the day
        
            struct tm *myTimeStruct = localtime(&myTime);
            int currentHour = myTimeStruct->tm_hour;
        
            if(millis() - lastRead > 1000)
            {
                lastRead = millis();
                sht3xData.temperature = (float)sht3x.getTemperatureF();
                sht3xData.humidity = (float)sht3x.getHumidityRH();
        
                // Check if current time is between 1400 (2 PM) and 1700 (5 PM)
                if(currentHour >= 14 && currentHour < 17)
                {
                    tempSum += sht3xData.temperature;
                    tempCount++;
                    if( tempCount > 0)
                    {
                        sht3xData.avgOATPreviousDay = tempSum / tempCount;
                    }
                    avgCalculatedForDay = false; // Reset the flag during this period
                }
                else if(currentHour >= 17 && !avgCalculatedForDay) // Past 1700 and we have readings
                {
                    if( tempCount > 0)
                    {
                        sht3xData.avgOATPreviousDay = tempSum / tempCount;
                        EEPROM.put(EEPROM_PREVIOUS_TEMP_LAST_DAY_ADDRESS, sht3xData.avgOATPreviousDay);     
                    }               
                    // Reset for the next day
                    tempSum = 0;
                    tempCount = 0;
                    avgCalculatedForDay = true; // Set the flag to indicate average is calculated
                }
            }
            if(avgCalculatedForDay)
            {
                EEPROM.get(EEPROM_PREVIOUS_TEMP_LAST_DAY_ADDRESS, sht3xData.avgOATPreviousDay);
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





