#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
#include "ADAFruitLogger.h"
#include "BME280.h"
#include <malloc.h>



#define RESET_FLAG_ADDRESS 0 // EEPROM address to store the reset flag

#define SEALEVELPRESSURE_HPA (1013.25)


//todo:
// 1. setup wifi
// 2. setup realtime clock
// 3. make webapp
// 4. iphone app
// 5. bluetooth
// 7. data logging
// 8. determine watering schedule
// 9. figure out how to deal with sensor errors
// 10. setup weather station/purchase a better one. 

cSoilSensor soilSensor;
cWIFIInterface wifiInterface;

sSoilSensorData soilSensorData;


int freeRAM() {
    struct mallinfo mi = mallinfo();
    return mi.fordblks;
}


unsigned long checkeHeapTime = 0;

cBME280 bme280;
void setup()
{
  Serial.begin(baud);
  pinMode(Valve1, OUTPUT);
  pinMode(Valve2, OUTPUT);
  pinMode(Valve3, OUTPUT);
  pinMode(sdChipSelect, OUTPUT);
  //digitalWrite(Valve1, HIGH);
  delay(1000);
  //digitalWrite(Valve1, LOW);
  delay(500);
  if (EEPROM.read(RESET_FLAG_ADDRESS) == 0) // Check if the reset flag is 0 (i.e., the software reset hasn't been performed yet)
  {
    Serial.println("Testing Software Reset");
    delay(1000);
    EEPROM.write(RESET_FLAG_ADDRESS, 1); // Set the reset flag to 1 (i.e., the software reset has been performed)
    softwareReset();
  }
  else
  {
    Serial.println("Reset already performed");
    EEPROM.write(RESET_FLAG_ADDRESS, 0);
  }
  //digitalWrite(Valve1, HIGH);
  delay(1000);
  digitalWrite(Valve1, LOW);


  //logger.setupLogger();
  if( logger.setupRTC() )
  {
    Serial.println("RTC setup successful.");
    Serial.print("Current time: ");
    Serial.println(logger.getExcelFormattedTime());
    Serial.print("Current Date: ");
    Serial.println(logger.getExcelFormattedDate());
  }
  else
  {
    Serial.println("RTC setup failed.");
    softwareReset();
  }
}

bool firstPass = true;
unsigned long epochTime = 0;

void loop()
{
  unsigned long epoch = 0;
  if(!firstPass && WiFi.status() == WL_CONNECTED && wifiInterface.CheckNtpTime(&epoch))
  {
    if (rtcFailed && logger.setupRTC())
    {
      logger.SetTime(epoch);
    }

    if (!rtcFailed)
    {
      logger.SetTime(epoch);
    }

    epochTime = logger.getUnixTime();
  }
  else
  {
    epochTime = logger.getUnixTime();
  }
  
  // If millis() is going to rollover in the next 24 hours
  if (millis() > ULONG_MAX - 86400000) {
      softwareReset();
  }    
  
  soilSensorData.epochTime = epochTime;
  bme280.runBME(&soilSensorData);

  soilSensor.runSoilSensor(&soilSensorData);

  time_t myTime = static_cast<time_t>(epochTime);

  wifiInterface.runWIFI(&soilSensorData, myTime);

  logger.RunLogger(&soilSensorData, WiFi.status() != WL_CONNECTED);

  if(WiFi.status() != WL_CONNECTED && rtcFailed)
  {
    softwareReset();
  }

  manageWateringValves(myTime, &soilSensorData);
  //printValues();

  //checkHeap();
  // if(millis() - checkeHeapTime > 1000 || checkeHeapTime == 0)
  // {
  //   checkeHeapTime = millis();
  //   Serial.print("Free Memory: ");
  //   Serial.println(freeRAM());
  // }

  firstPass = false;
}

void manageWateringValves(time_t myTime, sSoilSensorData* soilSensorData)
{
  static bool autoCycleActive = false;
  static time_t autoCycleStartTime = 0;

  static time_t displayData = myTime;

  struct tm *myTimeStruct = localtime(&myTime);
  bool startCycle1 = myTimeStruct->tm_hour == 6 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);
  bool startCycle2 = myTimeStruct->tm_hour == 7 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);


  bool needsWater = soilSensorData->soilMoisture < 20;

  if( difftime(myTime, displayData) > 60)
  {
    Serial.println();
    
    Serial.print("startCycle1: ");
    Serial.println(startCycle1);
    
    Serial.print("startCycle2: ");
    Serial.println(startCycle2);
    
    Serial.print("needsWater: ");
    Serial.println(needsWater);
    
    Serial.print("autoCycleActive: ");
    Serial.println(autoCycleActive);
    
    Serial.print("soilMoisture: ");
    Serial.println(soilSensorData->soilMoisture);

    Serial.print("autoCycleStartTime: ");

    struct tm *autoCycleStruct = localtime(&autoCycleStartTime);
    char timeStr[9]; // Buffer to hold the time string
    Serial.print("AutoCycle Start Time: ");
    sprintf(timeStr, "%02d:%02d:%02d", autoCycleStruct->tm_hour, autoCycleStruct->tm_min, autoCycleStruct->tm_sec);
    Serial.println(timeStr);
    
    tm *myTimeStruct = localtime(&myTime);
    Serial.print("Time: ");
    timeStr[9]; // Buffer to hold the time string
    sprintf(timeStr, "%02d:%02d:%02d", myTimeStruct->tm_hour, myTimeStruct->tm_min, myTimeStruct->tm_sec);
    Serial.println(timeStr);
    Serial.println();

    displayData = myTime;
  }

  if( (startCycle1 && !autoCycleActive) || (startCycle2 && !autoCycleActive) )
  {
    autoCycleActive = needsWater && gAutoWateringEnabled;
    autoCycleStartTime = millis() / 1000;
  }
  else if( (millis() / 1000 - autoCycleStartTime) > 60 * 10 )
  {
    autoCycleActive = false;
  }

  //indicate to the global state that the auto watering cycle is active
  gAutoWateringCycleOn = autoCycleActive;

  if( gManualWateringOn || autoCycleActive )
  {
    digitalWrite(Valve1, HIGH);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
    if( logger.getUnixTime() - gWateringTimeStart > gWateringDuration)
    {
      gManualWateringOn = false;
    }
  }
  else
  {
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
    gWateringTimeStart = logger.getUnixTime() - gWateringDuration;
  }

}

void printValues() {

  static int lastRead = millis();
  if(millis() - lastRead > 1000)
  {
    lastRead = millis();
    
    Serial.print("Temperature = ");
    Serial.print(soilSensorData.outsideAirTemp);
    Serial.println(" *F");

    Serial.print("Pressure = ");
    Serial.print(soilSensorData.baroPressure);


    Serial.print("Humidity = ");
    Serial.print(soilSensorData.outsideAirHumidity);
    Serial.println(" %");

    Serial.print("Soil Temperature = ");
    Serial.print(soilSensorData.soilTemperature);
    Serial.println(" *F");

    Serial.print("Soil Moisture = ");
    Serial.print(soilSensorData.soilMoisture);
    Serial.println(" %");

    Serial.print("Soil Electrical Conductivity = ");
    Serial.print(soilSensorData.soilElectricalConductivity);
    Serial.println(" uS/cm");

    Serial.print("Soil pH = ");
    Serial.print(soilSensorData.soilPh);
    Serial.println(" pH");
  }


}

void softwareReset() 
{
  // Trigger a software reset using the NVIC_SystemReset function
  NVIC_SystemReset();
}
