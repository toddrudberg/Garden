#include <Arduino.h>
#include <DFRobot_SHT3x.h>

// Initialize the sensor with I2C address 0x44 and reset pin 4 (change if necessary)
DFRobot_SHT3x sht3x(&Wire, 0x44, 4);

void setup() 
{
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Start serial communication at 9600 baud rate.
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("Serial communication started.");

  // Initialize the sensor.
  while (sht3x.begin() != 0) 
  {
    Serial.println("Failed to Initialize the chip, please confirm the wire connection");
    delay(1000);
  }

  Serial.println("Sensor initialized successfully.");
}

void loop() {
  // Uncomment the following code if you want to blink the built-in LED
  // digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second

  // Print temperature and humidity readings to the Serial Monitor.
  Serial.print("Ambient Temperature(°C/F): ");
  float tempC = sht3x.getTemperatureC();
  Serial.print(tempC);  // Get and print temperature in Celsius
  Serial.print(" C / ");
  float tempF = sht3x.getTemperatureF();
  Serial.print(tempF);  // Get and print temperature in Fahrenheit
  Serial.print(" F ");
  
  Serial.print("Relative Humidity(%RH): ");
  float humidity = sht3x.getHumidityRH();
  Serial.print(humidity);  // Get and print relative humidity
  Serial.println(" %RH");

  delay(2000);  // Wait for 2 seconds before taking the next reading
}
