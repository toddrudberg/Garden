//#include "ADAFruitLogger.h"
#include "IO.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

bool cAdafruitLogger::setupRTC()
{

    for( int i = 0; i < 10; i++)
    {
    if (!rtc.begin()) 
      {
        Serial.println("Couldn't find RTC");
        if(i == 9)
        {
          rtcFailed = true;
          return false;
        }
        delay(1000);
      }   
      else
      {
        Serial.println("RTC found!");
        break;
      }
    }

    // if (!rtc.initialized() || rtc.lostPower()) 
    // {
    //   Serial.println("May have lost power, let's set the time!");
    //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // }

    Serial.println("RTC is setup and running!");
    // When the RTC was stopped and stays connected to the battery, it has
    // to be restarted by clearing the STOP bit. Let's do this to ensure
    // the RTC is running.
    rtc.start();
    //rtc.adjust(DateTime(1033776000));
    return true;
}

void cAdafruitLogger::SetTime(unsigned long unixTime)
{
  // Get the current time from the RTC
  DateTime now = rtc.now();

  // Calculate the difference between the current time and unixTime
  long difference = labs(now.unixtime() - unixTime);
  // Serial.print("Current Time: ");
  // Serial.println(now.unixtime());
  // Serial.print("Unix Time: ");
  // Serial.println(unixTime);
  // Only adjust the RTC time if the difference is more than 10 seconds
  // Serial.print("Difference: ");
  // Serial.println(difference);
  if (difference > 5) 
  {
    rtc.adjust(DateTime(unixTime));
    Serial.print("RTC time adjusted by ");
    Serial.print(difference);
    Serial.println(" seconds.");
  }
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
      //DateStamp, TimeStamp, Epoch, OutsideAirTemp, OutsideHumidity, OutsideBaro, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh, Watering, WifiError, SDError, RTCFailed
      dataFile.println("DateStamp, TimeStamp, Epoch, OutsideAirTemp, OutsideHumidity, OutsideBaro, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh, Watering, WifiError, SDError, RTCFailed");
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
    dataFile.print(soilSensorData->epochTime);
    dataFile.print(", ");
    dataFile.print(soilSensorData->outsideAirTemp);
    dataFile.print(", ");
    dataFile.print(soilSensorData->outsideAirHumidity);
    dataFile.print(", ");
    dataFile.print(soilSensorData->baroPressure);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilTemperature);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilElectricalConductivity);
    dataFile.print(", ");
    dataFile.print(soilSensorData->soilMoisture);
    dataFile.print(", ");
    dataFile.println(soilSensorData->soilPh);
    dataFile.print(", ");
    dataFile.print(gWatering);
    dataFile.print(", ");
    dataFile.print(wifiConnectionFailed);
    dataFile.print(", ");
    dataFile.print(SD.exists(FileName));
    dataFile.print(", ");
    dataFile.println(rtcFailed);
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

