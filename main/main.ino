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
cAdafruitLogger logger;
sSoilSensorData soilSensorData;
void setup()
{
  Serial.begin(baud);
  //logger.setupLogger();
  if( logger.setupRTC() )
  {
    Serial.println("RTC setup successful.");
  }
  else
  {
    Serial.println("RTC setup failed.");
  }
}

void loop()
{
  soilSensor.runSoilSensor(&soilSensorData);
  //soilSensorData.timeStamp = gTimeString;
  wifiInterface.runWIFI(&soilSensorData);
  logger.RunLogger(&soilSensorData);
}
