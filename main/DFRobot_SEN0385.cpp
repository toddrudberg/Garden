
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
            static unsigned long lastRead = millis();
            static sSEN0385Data sht3xData;
            static float tempSum = 0; // Sum of temperatures
            static unsigned int tempCount = 0; // Count of temperature readings
            static bool avgTempRecorded = false;
        


            if(millis() - lastRead > 5000)
            {
                struct tm *myTimeStruct = localtime(&myTime);
                int currentHour = myTimeStruct->tm_hour;
                lastRead = millis();
                sht3xData.temperature = (float)sht3x.getTemperatureF();
                sht3xData.humidity = (float)sht3x.getHumidityRH();
        
                // Serial.println();
                // Serial.println("Debugging Temperature Reading");
                // Serial.print("Current Hour: ");
                // Serial.println(currentHour);
                // Serial.print("Temp Sum: ");
                // Serial.println(tempSum);
                // Serial.print("Temp Count: ");
                // Serial.println(tempCount);
                // Serial.print("Current Temp: ");
                // Serial.println(sht3xData.temperature);

                unsigned long epochTime = sensorData->epochTime;

                //Serial.print("Which state: ");
        
                // Check if current time is between 1400 (2 PM) and 1700 (5 PM)
                if(currentHour >= 14 && currentHour < 17)
                {
                    // Serial.println("Between 1400 and 1700");
                    tempSum += sht3xData.temperature;
                    tempCount++;
                    sht3xData.avgOATPreviousDay = tempSum / (float) tempCount;
                    avgTempRecorded = false;
                }
                else if(currentHour >= 17 && tempCount > 0 && !avgTempRecorded) // Past 1700 and we have readings
                {
                    // Serial.println("Past 1700 and we have readings");
                    sht3xData.avgOATPreviousDay = tempSum / (float) tempCount;
                    EEPROM.put(EEPROM_AVG_OAT_PREVIOUS_DAY_ADDRESS, sht3xData.avgOATPreviousDay);
                    // Reset for the next day
                    tempSum = 0;
                    tempCount = 0;
                    avgTempRecorded = true;
                    // Optionally, do something with averageTemp, like storing or displaying it
                }
                else if (!avgTempRecorded)
                {  // If it's not between 1400 and 1700, and we haven't recorded the average temperature yet
                    // Serial.println("Not between 1400 and 1700, and we haven't recorded the average temperature yet");
                    EEPROM.get(EEPROM_AVG_OAT_PREVIOUS_DAY_ADDRESS, sht3xData.avgOATPreviousDay);
                }
                // Serial.println();
                // Serial.print("Avg Temp Recorded: ");
                // Serial.println(sht3xData.avgOATPreviousDay);
                // Serial.println();

                sensorData->outsideAirTemp = sht3xData.temperature;
                sensorData->outsideAirHumidity = sht3xData.humidity;
                sensorData->baroPressure = sht3xData.avgOATPreviousDay;
                sensorData->avgOATPreviousDay = sht3xData.avgOATPreviousDay;
            }

            break;        
          }
        default:
            processState = 0;
            break;
    }
}





