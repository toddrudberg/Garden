#include <max6675.h>
#include <SPI.h>
#include <SD.h>

const int SD_ChipSelect = 10; 
const int ThermoDataOutput = 4;
const int ThermoChipSelect = 5;
const int ThermoClock = 6;
const char* FileName = "Log.csv";

MAX6675 thermocouple(ThermoClock, ThermoChipSelect, ThermoDataOutput);

struct TempReading 
{
  unsigned int timeStamp;
  double outsideAirTemp;
  double soilMoisture;
  double soilTemperature;
  double soilElectricalConductivity;
  double soilHumidity;
  double soilPh;
};

TempReading tempReadings[10]; 
int tempIndex = 0; 

void setup() 
{
  pinMode(SD_ChipSelect, OUTPUT);

  SPI.begin();
  Serial.begin(9600); 

  if (!SD.begin(SD_ChipSelect)) 
  {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialization successful!");

  delay(3000);
}

void loop() 
{
  Serial.print("Time = ");
  Serial.print( millis() / 1000.0 );
  Serial.print("s - idx = ");
  Serial.print(tempIndex);
  Serial.print(" - F = ");
  Serial.println(thermocouple.readFahrenheit());
  double fahrenheit = thermocouple.readFahrenheit();
  if (tempIndex < static_cast<int>(sizeof(tempReadings) / sizeof(TempReading)))
  {
    tempReadings[tempIndex].outsideAirTemp = fahrenheit;
    tempReadings[tempIndex].timeStamp = static_cast<unsigned int>(millis() / 1000.0);
    tempIndex++;
  }
  if( tempIndex == static_cast<int>(sizeof(tempReadings) / sizeof(TempReading)))
  {
    writeTempReadingsToFile();
    tempIndex = 0;
  }
  
  delay(1000); 
}

void writeTempReadingsToFile() 
{
  if (!SD.exists(FileName)) 
  {
    File dataFile = SD.open(FileName, FILE_WRITE);
    if (dataFile) 
    {
      dataFile.println("TimeStamp, OutsideAirTemp, SoilMoisture, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh");
      dataFile.close();
    } 
    else 
    {
      Serial.println("File Creation Error!");
      return;
    }
  }

  File dataFile = SD.open(FileName, FILE_WRITE);
  if (dataFile) 
  {
    for (int i = 0; i < static_cast<int>(sizeof(tempReadings) / sizeof(TempReading)); i++) 
    {
      dataFile.print(tempReadings[i].timeStamp);
      dataFile.print(", ");
      dataFile.print(tempReadings[i].outsideAirTemp);
      dataFile.print(", ");
      dataFile.print(tempReadings[i].soilMoisture);
      dataFile.print(", ");
      dataFile.print(tempReadings[i].soilTemperature);
      dataFile.print(", ");
      dataFile.print(tempReadings[i].soilElectricalConductivity);
      dataFile.print(", ");
      dataFile.print(tempReadings[i].soilHumidity);
      dataFile.print(", ");
      dataFile.println(tempReadings[i].soilPh);
    }
    dataFile.close();
    Serial.print("Temperature data written to ");
    Serial.println(FileName);
  } 
  else 
  {
    Serial.println("File write error!");
  }
}