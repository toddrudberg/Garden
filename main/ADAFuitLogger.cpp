//#include "ADAFruitLogger.h"
#include "IO.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

bool cAdafruitLogger::setupRTC()
{

    if (!rtc.begin()) 
    {
      Serial.println("Couldn't find RTC");
      return false;
    }   

    if (!rtc.initialized() || rtc.lostPower()) 
    {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  
    Serial.println("RTC is setup and running!");
    // When the RTC was stopped and stays connected to the battery, it has
    // to be restarted by clearing the STOP bit. Let's do this to ensure
    // the RTC is running.
    rtc.start();
    return true;
}

uint32_t cAdafruitLogger::getUnixTime() {
  DateTime now = rtc.now();
  return now.unixtime();
}

char* cAdafruitLogger::getExcelFormattedDate() {
  static char dateString[11]; // Buffer to hold the date string
  DateTime now = rtc.now();

  // Format the date into the string
  sprintf(dateString, "%04d-%02d-%02d", now.year(), now.month(), now.day());

  return dateString;
}

char* cAdafruitLogger::getExcelFormattedTime() {
  static char timeString[9]; // Buffer to hold the time string
  DateTime now = rtc.now();

  // Format the time into the string
  sprintf(timeString, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  return timeString;
}

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
                soilSensorData->dateStamp = getExcelFormattedDate();
                soilSensorData->timeStamp = getExcelFormattedTime();
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
      dataFile.println("DateStamp, TimeStamp, OutsideAirTemp, OutsideHumidity, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh");
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
    dataFile.print(soilSensorData->dateStamp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->timeStamp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->outsideAirTemp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->outsideAirHumidity);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilTemperature);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilElectricalConductivity);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilMoisture);
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

