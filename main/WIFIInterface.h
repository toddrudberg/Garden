#ifndef WIFIINTERFACE_H
#define WIFIINTERFACE_H

#include "IO.h"
#include <WiFi.h>
#include <NTPClient.h>

class cWIFIInterface
{
public:
    cWIFIInterface();
    bool CheckNtpTime(unsigned long *epoch);
    void runWIFI(sSoilSensorData* soilSensorData, time_t epochTime);

private:
    void update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime);
    void read_dropServer();
    void checkWIFI(sSoilSensorData* soilSensorData, time_t epochTime);
    bool setupWIFI();
    void setManualWaterStatus(bool request);
    void setAutolWaterStatus(bool request);


    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "Turkey Point";
    const char* password = "gobblegobble";
    WiFiServer server;
};

#endif // WIFIINTERFACE_H