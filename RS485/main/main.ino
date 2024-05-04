#include "IO.h"
#include "SoilSensor.h"
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

void setup()
{
  Serial.begin(baud);
  soilSensor.SetupSoilSensor();
}


void loop()
{
  soilSensor.CheckSoilSensor();
}
