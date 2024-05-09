#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
#include "ADAFruitLogger.h"
#include "BME280.h"
#include <malloc.h>

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
  }

  pinMode(Valve1, OUTPUT);
  pinMode(Valve2, OUTPUT);
  pinMode(Valve3, OUTPUT);
  pinMode(sdChipSelect, OUTPUT);


}

bool firstPass = true;
void loop()
{

bme280.runBME(&soilSensorData);
soilSensor.runSoilSensor(&soilSensorData);
wifiInterface.runWIFI(&soilSensorData);
logger.RunLogger(&soilSensorData);


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
  if(millis() - checkeHeapTime > 1000 || checkeHeapTime == 0)
  {
    checkeHeapTime = millis();
    Serial.print("Free Memory: ");
    Serial.println(freeRAM());
  }

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
