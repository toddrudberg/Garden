#ifndef WIFIINTERFACE_H
#define WIFIINTERFACE_H
#include <WiFi.h>
#include <NTPClient.h>

class cWIFIInterface
{
public:
  cWIFIInterface();
  void setupWIFI();
  void checkWIFI();


private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    const char* ssid = "CenturyLink2286";
    const char* password = "vvgs3nsmrw2549";
    WiFiServer server;
};

#endif // WIFIINTERFACE_H