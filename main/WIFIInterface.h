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

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "CenturyLink2286";
    const char* password = "vvgs3nsmrw2549";
    WiFiServer server;
};

#endif // WIFIINTERFACE_H