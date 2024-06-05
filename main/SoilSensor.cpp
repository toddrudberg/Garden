#include "IO.h"
#include "SoilSensor.h"

const int soilSensorArraySize = 10;
sSoilSensorData soilSensorDataArray[soilSensorArraySize];

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
      sSoilSensorData newSoilSensorData = *soilSensorData;

      if( checkSoilSensor(&newSoilSensorData) )
      {
        *soilSensorData = fillSoilSensorDataArray(soilSensorDataArray, newSoilSensorData);
        // Serial.print(soilSensorData->soilMoisture);
        // Serial.print(",");
        // Serial.print(soilSensorData->soilTemperature);
        // Serial.print(",");
        // Serial.println(soilSensorData->soilPh);
      }

      break;
    }
  }
}

void cSoilSensor::setupSoilSensor()
{
  mySerial.begin(9600);
  pinMode(rs485TxEnable, OUTPUT);
  digitalWrite(rs485TxEnable, LOW);  
}

sSoilSensorData cSoilSensor::fillSoilSensorDataArray(sSoilSensorData* soilSensorDataArray, sSoilSensorData newSoilSensorData) 
{
  static int currentIndex = 0;
  static bool bufferFull = false;
  static double soilMoistureAvg = 25; //intialize to 25% moisture

  // Add the new object to the array at the current index
  soilSensorDataArray[currentIndex] = newSoilSensorData;

  // Increment the current index, and roll over if it reaches the end of the array
  if(currentIndex == soilSensorArraySize - 1) bufferFull = true;
  currentIndex = (currentIndex + 1) % soilSensorArraySize;

  // Calculate the average of the data collected
  double outsideAirTempSum = 0;
  double outsideAirHumiditySum = 0;
  double soilTemperatureSum = 0;
  double soilElectricalConductivitySum = 0;
  double soilMoistureSum = 0;
  double soilPhSum = 0;
  int bufferLength = bufferFull ? soilSensorArraySize : currentIndex;
  for (int i = 0; i < bufferLength; i++) 
  {
    outsideAirTempSum += soilSensorDataArray[i].outsideAirTemp;
    outsideAirHumiditySum += soilSensorDataArray[i].outsideAirHumidity;
    soilTemperatureSum += soilSensorDataArray[i].soilTemperature;
    soilElectricalConductivitySum += soilSensorDataArray[i].soilElectricalConductivity;

    double shTest = static_cast<double>(soilSensorDataArray[i].soilMoisture);
    if( shTest < 0 || shTest > 100)
    {
      shTest = soilMoistureAvg;
    }
    else if(fabs(shTest - soilMoistureAvg) > 10)
    {
      shTest = soilMoistureAvg;
    }

    soilMoistureSum += shTest;//soilSensorDataArray[i].soilMoisture;
    soilPhSum += soilSensorDataArray[i].soilPh;
  }
  double outsideAirTempAvg = outsideAirTempSum / bufferLength;
  double outsideAirHumidityAvg = outsideAirHumiditySum / bufferLength;
  double soilTemperatureAvg = soilTemperatureSum / bufferLength;
  double soilElectricalConductivityAvg = soilElectricalConductivitySum / bufferLength;
  soilMoistureAvg = soilMoistureSum / bufferLength;
  double soilPhAvg = soilPhSum / bufferLength;

  // Create a new object to hold the average data
  sSoilSensorData avgSoilSensorData;
  avgSoilSensorData.dateStamp = newSoilSensorData.dateStamp;
  avgSoilSensorData.timeStamp = newSoilSensorData.timeStamp;
  avgSoilSensorData.outsideAirTemp = static_cast<float>(outsideAirTempAvg);
  avgSoilSensorData.outsideAirHumidity = static_cast<float>(outsideAirHumidityAvg);
  avgSoilSensorData.soilTemperature = static_cast<float>(soilTemperatureAvg);
  avgSoilSensorData.soilElectricalConductivity = static_cast<float>(soilElectricalConductivityAvg);
  avgSoilSensorData.soilMoisture = static_cast<float>(soilMoistureAvg);
  avgSoilSensorData.soilPh = static_cast<float>(soilPhAvg);

  return avgSoilSensorData;
}

bool cSoilSensor::checkSoilSensor(sSoilSensorData* soilSensorData)
{
  static int processStep = 0;
  static unsigned long startTime = millis();  
  int temp;
  bool processedNewData = false;
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
        unsigned long loopStartTime = millis();
        unsigned long timeout = 1000; // timeout after 1000 milliseconds        
        while (mySerial.available() > 0) 
        {
          if (millis() - loopStartTime > timeout)
          {
              break; // exit the loop if timeout is reached
          }          
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
        processedNewData = true;


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
  return processedNewData;
}