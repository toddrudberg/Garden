#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
#include "ADAFruitLogger.h"
#include "DFRobot_SEN0385.h"
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

//cBME280 bme280;
cSEN0385 DFRSEN0385;
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
    Serial.println(logger.getExcelFormattedTime(0));
    Serial.print("Current Date: ");
    Serial.println(logger.getExcelFormattedDate(0));
  }
  else
  {
    Serial.println("RTC setup failed.");
    softwareReset();
  }
}

bool firstPass = true;

void loop()
{
  static unsigned long epochTime = 0;
  // unsigned long epoch = 0;
  // if(!firstPass && WiFi.status() == WL_CONNECTED && wifiInterface.CheckNtpTime(&epoch))
  // {
  //   if (rtcFailed && logger.setupRTC())
  //   {
  //     logger.SetTime(epoch);
  //   }

  //   if (!rtcFailed)
  //   {
  //     logger.SetTime(epoch);
  //   }

  //   epochTime = logger.getUnixTime();
  // }
  // else
  // {
  //   epochTime = logger.getUnixTime();
  // }
  
  wifiInterface.CheckNtpTime(&epochTime);
  logger.SetTime(epochTime);

  // If millis() is going to rollover in the next 24 hours
  if (millis() > ULONG_MAX - 86400000) {
      softwareReset();
  }    
  
  soilSensorData.epochTime = epochTime;
  //bme280.runBME(&soilSensorData);
  DFRSEN0385.run385(&soilSensorData);

  soilSensor.runSoilSensor(&soilSensorData);

  time_t myTime = static_cast<time_t>(epochTime);

  wifiInterface.runWIFI(&soilSensorData, myTime);

  logger.RunLogger(&soilSensorData, WiFi.status() != WL_CONNECTED, myTime);

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
  static unsigned long autoCycleStartTime = 0;
  static unsigned long wateringDuration = 0; // Default duration
  static time_t displayData = myTime;

  struct tm *myTimeStruct = localtime(&myTime);
  bool startCycle09 = myTimeStruct->tm_hour == 9 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);
  bool startCycle14 = myTimeStruct->tm_hour == 14 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);
  bool startCycle17 = myTimeStruct->tm_hour == 17 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);

  float soilMoisture = soilSensorData->soilMoisture;


  if( difftime(myTime, displayData) > 60)
  {
    Serial.println();
    
    Serial.print("startCycle1: ");
    Serial.println(startCycle09);
    
    Serial.print("startCycle2: ");
    Serial.println(startCycle14);

        
    Serial.print("startCycle17: ");
    Serial.println(startCycle17);

    
    Serial.print("autoCycleActive: ");
    Serial.println(autoCycleActive);
    
    Serial.print("soilMoisture: ");
    Serial.println(soilSensorData->soilMoisture);

    Serial.print("autoCycleStartTime: ");


    // output the autoCycleStartTime
    Serial.print("autoCycleStartTime: ");
    Serial.println(autoCycleStartTime);
    //output the wateringDuration
    Serial.print("wateringDuration: ");
    Serial.println(wateringDuration);
    //output the remainting time:
    Serial.print("Remaining Time: ");
    unsigned long remainingTime = autoCycleActive ? wateringDuration - (millis() / 1000 - autoCycleStartTime) : 0;
    Serial.println(remainingTime);
    Serial.print("Current Time: ");
    Serial.println(millis() / 1000);

    tm *myTimeStruct = localtime(&myTime);
    Serial.print("Time: ");
    char timeStr[10]; // Correctly declare the buffer to hold the time string
    sprintf(timeStr, "%02d:%02d:%02d", myTimeStruct->tm_hour, myTimeStruct->tm_min, myTimeStruct->tm_sec);
    Serial.println(timeStr);
    Serial.println();


    displayData = myTime;
  }

  static int autoCycleStep = 0;
  switch (autoCycleStep)
  {
  // check if the cycle should start
  case 0:
    if (startCycle09 || startCycle14 || startCycle17) 
    {
      autoCycleStep++;
    }
    break;

  //calculate the watering duration
  case 1:
    autoCycleStartTime = millis() / 1000;

    if (startCycle09 && soilMoisture < 30.0) 
    {
      float lowMoisture = 22.0; // 22% soil moisture - moisture is low
      float highMoisture = 30.0; // 30% soil moisture - moisture is high
      float lowDuration = 25.0; // 25 minutes if soil moisture is 22%
      float highDuration = 5.0; // 5 minutes if soil moisture is 28%
      const float m = (lowDuration - highDuration) / (lowMoisture - highMoisture);
      const float b = lowDuration - m * lowMoisture; 
      float wateringTime = m * soilMoisture + b;
      wateringDuration = 60 * wateringTime;
    } 
    else if ((startCycle14 || startCycle17) && soilMoisture < 25.0) 
    {
      wateringDuration = 60 * 5; // For both startCycle14 and startCycle17
    }
    else 
    {
      wateringDuration = 0;
    }
    autoCycleStep++;
    break;

  // wait for the duration to expire
  case 2:
    autoCycleActive = true;
    if ( (millis() / 1000 - autoCycleStartTime) > wateringDuration) 
    {
      autoCycleActive = false;
      autoCycleStep++;
    }
    break;

  // ensure the startCycle flags are not set
  case 3:
    if(!(startCycle09 || startCycle14 || startCycle17))
    {
      autoCycleStep = 0;
    }
    break;

  default:
    autoCycleStep = 0;
    break;
  }


  //indicate to the global state that the auto watering cycle is active
  gAutoWateringCycleOn = autoCycleActive;
  unsigned long ulEpochTime = USERTC ? logger.getUnixTime() : static_cast<unsigned long>(myTime);
  if( gManualWateringOn || autoCycleActive )
  {
    digitalWrite(Valve1, HIGH);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
    if( ulEpochTime - gWateringTimeStart > gWateringDuration)
    {
      gManualWateringOn = false;
    }
  }
  else
  {
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
    gWateringTimeStart = ulEpochTime - gWateringDuration;
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
