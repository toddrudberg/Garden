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
    void update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime);
    void checkWIFI(sSoilSensorData* soilSensorData, time_t epochTime);
    void runWIFI(sSoilSensorData* soilSensorData, time_t epochTime);
    bool CheckNtpTime(unsigned long *epoch);

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "Turkey Point";
    const char* password = "gobblegobble";
    WiFiServer server;
};

#endif // WIFIINTERFACE_H