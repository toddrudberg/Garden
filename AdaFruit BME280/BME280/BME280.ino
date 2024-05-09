#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

Adafruit_BME280 bme; // I2C
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("insetup");
  if (!bme.begin(0x76)) {  // Make sure the address is correct
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (!rtc.begin()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.println(now.day(), DEC);

  delay(1000);
}
