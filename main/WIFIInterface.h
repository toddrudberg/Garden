#ifndef WIFIINTERFACE_H
#define WIFIINTERFACE_H
#include "IO.h"
#include <WiFi.h>
#include <NTPClient.h>

class cWIFIInterface
{
public:
    cWIFIInterface();
    bool setupWIFI();
    void checkWIFI(sSoilSensorData* soilSensorData);
    void runWIFI(sSoilSensorData* soilSensorData);
    void CheckNtpTime();

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "Turkey Point";
    const char* password = "gobblegobble";
    WiFiServer server;
};

#endif // WIFIINTERFACE_H