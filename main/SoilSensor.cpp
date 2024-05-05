#include "IO.h"
#include "SoilSensor.h"

void cSoilSensor::runSoilSensor(sSoilSensorData* soilSensorData)
{
  static int state = 0;
  switch(state)
  {
    case 0:
    {
      setupSoilSensor();
      state++;
      break;
    }
    case 1:
    {
      checkSoilSensor(soilSensorData);
      break;
    }
  }
}

void cSoilSensor::setupSoilSensor()
{
  mySerial.begin(baud);
  pinMode(rs485TxEnable, OUTPUT);
  digitalWrite(rs485TxEnable, LOW);  
}

void cSoilSensor::checkSoilSensor(sSoilSensorData* soilSensorData)
{
  static int processStep = 0;
  static unsigned long startTime = millis();  
  int temp;
  switch(processStep)
  {
    case 0:
    {
      digitalWrite(rs485TxEnable, HIGH);    //Enable high, RS485 shield waiting to transmit data
      for (int i = 0; i < 8; i++) 
      {
        mySerial.write(instructionToSoilSensor[i]);
      }
      digitalWrite(rs485TxEnable, LOW);    //Enable low, RS485 shield waiting to receive data
      startTime = millis();
      processStep++;
      break;
    }
    case 1:
    {
      if(millis() - startTime > 100) //10ms isn't enough time and we aren't in a hurry.  so 100ms. 
      {
        startTime = millis();
        processStep++;
      }
      break;
    }
    case 2:
    {
      int i = 0;
      uint8_t data[100] = { 0 };
      if(mySerial.available())
      {
        while (mySerial.available() > 0) 
        {
          int temp = mySerial.read();
          data[i++] = temp;
        }

        int offset = i - 10;
        float humidity = ((data[0+offset] << 8) + data[1+offset]) / 10.00;
        float tempC = ((data[2+offset] << 8) + data[3+offset]) / 10.00;
        float ecValue = ((data[4+offset] << 8) + data[5+offset]);
        float phValue = ((data[6+offset] << 8) + data[7+offset]) /10.00;
        float tempF = tempC * 9.0/5.0 + 32.0;
        soilSensorData->soilMoisture = humidity;
        soilSensorData->soilTemperature = tempF;
        soilSensorData->soilElectricalConductivity = ecValue;
        soilSensorData->soilPh = phValue;

// struct sSoilSensorData
// {
//   unsigned int timeStamp;
//   double outsideAirTemp;
//   double soilMoisture;x
//   double soilTemperature;x
//   double soilElectricalConductivity;x
//   double soilHumidity;
//   double soilPh;
// };

        Serial.print(humidity);
        Serial.print(",");
        Serial.print(tempF);
        Serial.print(",");
        Serial.println(ecValue);

        startTime = millis();
        processStep++;
      }
      if( millis() - startTime > 3000)
      {
        Serial.println("timeout");
        processStep = 0;
      }
      break;
    }
    case 3:
    {
      if( millis() - startTime > msBetweenReads)
      {
        processStep = 0;
      }
      break;
    }
    default:
    {
      processStep = 0;
      break;
    }
  }
}