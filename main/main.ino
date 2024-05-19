#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
#include "ADAFruitLogger.h"
#include "BME280.h"
#include <malloc.h>
#include <EEPROM.h>

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
  digitalWrite(Valve1, HIGH);
  delay(1000);
  digitalWrite(Valve1, LOW);
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
  digitalWrite(Valve1, HIGH);
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
    if(!firstPass && !wifiConnectionFailed && wifiInterface.CheckNtpTime(&epoch))
    {
      if(rtcFailed)
      {
        epochTime = epoch;
      }
      else 
      {
        logger.SetTime(epoch);
        epochTime = logger.getUnixTime();
      }
    }
    else
    {
      epochTime = logger.getUnixTime();
    }
    

    bme280.runBME(&soilSensorData);

    soilSensor.runSoilSensor(&soilSensorData);

    time_t myTime = static_cast<time_t>(epochTime);
    wifiInterface.runWIFI(&soilSensorData, myTime);

    logger.RunLogger(&soilSensorData);

  if(wifiConnectionFailed && rtcFailed)
  {
    softwareReset();
  }



  if( gWatering )
  {
    digitalWrite(Valve1, HIGH);
    digitalWrite(Valve2, HIGH);
    digitalWrite(Valve3, HIGH);
    if( logger.getUnixTime() - gWateringTimeStart > gWateringDuration)
    {
      gWatering = false;
    }
  }
  else
  {
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
    gWateringTimeStart = logger.getUnixTime() - gWateringDuration;
  }

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
