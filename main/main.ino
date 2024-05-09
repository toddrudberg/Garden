#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
#include "ADAFruitLogger.h"
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
}

void loop()
{

  soilSensor.runSoilSensor(&soilSensorData);
  //soilSensorData.timeStamp = gTimeString;
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

}
