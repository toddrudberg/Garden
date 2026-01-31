# Garden
We want to control our sprinkler system for our garden. 

## Build Environment (Arduino)

This sketch is known-good with:

- **Board:** Arduino UNO R4 WiFi
- **Arduino core / platform:** `arduino:renesas_uno`
- **Core version tested:** 1.4.1 (see Arduino IDE → Boards Manager)
- **WiFi library:** `WiFiS3` (ships with the UNO R4 core)

### Why board selection matters
The Arduino build system pulls some libraries from the board core package.  
If you select a different board, you may compile against a different set of core libraries (or different versions).

Example (UNO R4 WiFi core library path):
`Arduino15/packages/arduino/hardware/renesas_uno/1.4.1/libraries/...`

### Third-party libraries
This project depends on third-party libraries that may not be reliably discoverable via Arduino Library Manager.

#### DFRobot_SHT3x
This repo includes (or requires) the `DFRobot_SHT3x` library.

**Install option (recommended):**
- Copy the folder from this repo into your Arduino libraries folder:
  - macOS: `~/Documents/Arduino/libraries/`
  - Windows: `Documents\Arduino\libraries\`

After copying, restart Arduino IDE.

> Note: The Arduino build log prints lines like `Using library ... in folder: ...`.
> If your paths differ significantly, you are likely compiling against a different board core or a different library install.

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

