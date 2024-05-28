# Soil Sensor Data Logger

This project logs soil sensor data to an SD card using an Arduino and the Adafruit SD library.

## Features

- Logs the following soil sensor data:
    - Date and time stamp
    - Epoch time
    - Outside air temperature
    - Outside air humidity
    - Barometric pressure
    - Soil temperature
    - Soil electrical conductivity
    - Soil moisture
    - Soil pH

Logging Header:
DateStamp, TimeStamp, Epoch, OutsideAirTemp, OutsideHumidity, OutsideBaro, SoilTemperature, SoilElectricalConductivity, SoilHumidity, SoilPh, Watering, WifiError, SDError, RTCFailed

DateStamp, char*
TimeStamp, char*
Epoch, unsigned long
OutsideAirTemp, float
OutsideHumidity, float
OutsideBaro, float
SoilTemperature, float
SoilElectricalConductivity, float
SoilHumidity, float
SoilPh, float
Watering, bool
WifiError, bool
SDError, bool
RTCFailed, bool

- Writes data to a file on an SD card in CSV format.

## Dependencies

- [Adafruit SD library](https://github.com/adafruit/SD)

## Usage

1. Connect your soil sensor and SD card module to your Arduino according to the wiring diagram provided in the documentation.

2. Upload the Arduino sketch to your Arduino.

3. The Arduino will start logging soil sensor data to the SD card.

## Troubleshooting

If you encounter any issues, check the serial monitor for error messages. Common issues include SD card write errors and file creation errors.

## Contributing

Contributions are welcome. Please open an issue to discuss your idea before making a pull request.

## License

This project is licensed under the MIT License. See the LICENSE file for details.