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
    bool update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime);
    bool read_dropServer(int requestType);
    void checkWIFI(sSoilSensorData* soilSensorData, time_t epochTime);
    bool setupWIFI();
    void setManualWaterStatus(bool request);
    void setAutolWaterStatus(bool request);
    bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime);
    bool read_dropServer(bool* autoWateringRequest, bool* manualWaterOverrideRequest, bool* aRefreshRequest);

    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "Turkey Point";
    const char* password = "gobblegobble";
    //char remoteServer[] = "192.168.1.31"; // address of your server
    const char* remoteServer = "64.23.202.34"; // address of your server
    const int remoteServerPort = 3000; // port of your server
    WiFiServer server;
};

#endif // WIFIINTERFACE_H