# Garden
We want to control our sprinkler system for our garden. 

# Inputs:
1 - DFRobot Soil Moisture, Temperature, Electrical Conductivity & PH
2 - outside air temp & humidity - AdaFruit Adafruit BME280 I2C or SPI Temperature Humidity Pressure Sensor
3 - RS485 Shield, DFRobot
4 - Adafruit Assembled Data Logging Shield for Arduino

# Outputs:
1 - 3 digital for water solonoid

# Mounting:
We'll need an Arduino Board mount
The shield stack will include:
 - Arduino R4 WIFI
 - RS485 Sheild
 - MicroSD/RTC Shield
 - Breakout Board

 We need a spot for the BME 280 Temp/Hum/Pressure sensor
 We need 3 spots for relay boards

# Zones
## Garden raised beds - mini sprinklers
we'll measure the soil for these and control water carefully
## Flower Patch - sprinklers
timer open loop
## Trees - drip lines
timer open loop

