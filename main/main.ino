#include "IO.h"
#include "SoilSensor.h"
#include "WIFIInterface.h"
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

void setup()
{
  Serial.begin(baud);
  Serial.println("Setting up soil sensor...");
  soilSensor.setupSoilSensor();
  Serial.println("Setting up WIFI...");
  wifiInterface.setupWIFI();

  Serial.println("Setup complete, beginning big loop...");
}


void loop()
{
  soilSensor.checkSoilSensor();
  wifiInterface.checkWIFI();
}
