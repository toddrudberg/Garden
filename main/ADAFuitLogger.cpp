#include "ADAFruitLogger.h"

void cAdafruitLogger::RunLogger(sSoilSensorData* soilSensorData)
{
    static int step = 0;
    const int timeDelay = 5000;
    static unsigned long startTime = millis() + timeDelay;
    switch(step)
    {
        case 0:
        {
            if(((millis() - startTime) > timeDelay) && setupLogger())
            {
                step++;
                startTime = millis();
            }
            break;
        }
        case 1:
        {
            if( millis() - startTime > timeDelay)
            {
                startTime = millis();
                if( !writeData(soilSensorData))
                {
                    step = 0;
                    Serial.println("Error writing data to SD card.");
                }
                startTime = millis();
            }
            break;
        }
    }
}

bool cAdafruitLogger::setupLogger()
{
    pinMode(sdChipSelect, OUTPUT);
    //SPI.begin();
    if (!SD.begin(sdChipSelect))
    {
        Serial.println("SD initialization failed!");
        return false;
    }
    Serial.println("SD initialization done.");
    return true;
}

bool cAdafruitLogger::writeData(sSoilSensorData* soilSensorData)
{
  if (!SD.exists(FileName)) 
  {
    File dataFile = SD.open(FileName, FILE_WRITE);
    if (dataFile) 
    {
      dataFile.println("TimeStamp, OutsideAirTemp, OutsideHumidity, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh");
      dataFile.close();
    } 
    else 
    {
      Serial.println("File Creation Error!");
      return false;
    }
  }
  File dataFile = SD.open(FileName, FILE_WRITE);
  if (dataFile) 
  {
    dataFile.print(soilSensorData->timeStamp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->outsideAirTemp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilMoisture);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilTemperature);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilElectricalConductivity);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilHumidity);
    dataFile.print(", ");
    dataFile.println(soilSensorData->soilPh);
    dataFile.close();
    Serial.print("Temperature data written to ");
    Serial.println(FileName);
    return true;
  } 
  else 
  {
    Serial.println("File write error!");
    return false;
  }
}

