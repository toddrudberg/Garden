#include "BME280.h"

Adafruit_BME280 bme; // I2C

void cBME280::runBME(sSoilSensorData* sensorData)
{
    static int processState = 0;

    switch (processState)
    {
        case 0:
        {
            static int startState = 0;
            static unsigned long timeOut = millis();
            switch(startState)
            {
                case 0:
                    if(!startBME())
                    {
                        startState++;
                        timeOut = millis();
                    }
                    else 
                    {
                        Serial.println("BME280 started successfully.");
                        processState++;
                    }
                    break;
                case 1:
                    if(millis() - timeOut > 1000)
                    {
                        startState = 0;
                    }
                    break;
            }
            break;
        }
        case 1:
        {
            static int lastRead = millis();
            static sBME280Data bmeData;
            if(millis() - lastRead > 1000)
            {

                lastRead = millis();
                bmeData.temperature = (float)(bme.readTemperature() * 9.0 / 5.0 + 32.0);
                bmeData.pressure = bme.readPressure() * 0.02953 / 100.0F;
                bmeData.humidity = (float)bme.readHumidity();
            }
            sensorData->outsideAirTemp = bmeData.temperature;
            sensorData->outsideAirHumidity = bmeData.humidity;
            sensorData->baroPressure = bmeData.pressure ;
            break;
        }
        default:
            processState = 0;
            break;
    }
}

bool cBME280::startBME() {
    unsigned status;
    // default settings
    status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
}

sBME280Data cBME280::readBME() 
{
    sBME280Data data;

    data.temperature = bme.readTemperature() * 9.0 / 5.0 + 32.0;
    data.pressure = bme.readPressure() / 100.0F;
    data.humidity = bme.readHumidity();
    return data;
}