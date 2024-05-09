#include "WIFIInterface.h"

cWIFIInterface::cWIFIInterface() : timeClient(ntpUDP, "pool.ntp.org", -25200), server(80) 
{
}

void cWIFIInterface::runWIFI(sSoilSensorData* soilSensorData)
{
    static int state = 0;
    switch(state)
    {
        case 0:
        {
            static int timeOut = millis();
            static bool connectionFailed = false;
            if( !connectionFailed && setupWIFI())
            {
                // If successfully connected to WiFi, print IP address
                Serial.println("Connected to WiFi");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());

                // Start the web server
                server.begin();
                Serial.println("Web server started");  
                state++;
            }
            else
            {
                connectionFailed = true;
                if(millis() - timeOut > 5000)
                {
                    Serial.println("WIFI setup failed, retrying...");
                    timeOut = millis();
                    connectionFailed = false;
                }
            }
            break;
        }
        case 1:
        {
            static int timeOut = millis();
            if( millis() - timeOut > 250)
            {
                timeOut = millis();
                checkWIFI(soilSensorData);
            }
            break;
        }
    }
   
}

bool cWIFIInterface::setupWIFI()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Set static IP address
    IPAddress ip(192, 168, 1, 9); // Replace with your desired IP
    IPAddress gateway(192, 168, 1, 1); // Replace with your gateway address
    IPAddress subnet(255, 255, 255, 0); // Replace with your subnet mask
    IPAddress dns(8, 8, 8, 8); // Optional: Replace with your preferred DNS server's IP

    WiFi.config(ip, dns, gateway, subnet);
    
    WiFi.begin(ssid, password);
    return WiFi.status() == WL_CONNECTED;  
}

void cWIFIInterface::CheckNtpTime()
{
    static unsigned long lastTime = 0;
    static unsigned long startTime = millis();
    static unsigned long lastNtpTime = 0;
    static String timeString = {"12:00:00"};

    // syncronizing time with NTP server, this will need to move to rtc later. 
    if (millis() - lastTime > 1000) 
    {
        lastTime = millis();
        if (timeClient.update())
        {
            startTime = millis() - timeClient.getSeconds() * 1000;
            lastNtpTime = timeClient.getHours() * 3600 + timeClient.getMinutes() * 60 + timeClient.getSeconds();
            //Serial.println(timeClient.getFormattedTime());
            timeString = String(timeClient.getFormattedTime());
            //timeClient.getFormattedTime();
        } 
        else 
        {
            unsigned long elapsedSeconds = (millis() - startTime) / 1000;
            unsigned long currentTime = lastNtpTime + elapsedSeconds;
            unsigned int seconds = currentTime % 60;
            unsigned int minutes = (currentTime / 60) % 60;
            unsigned int hours = (currentTime / 3600) % 24;
            char time_string[9];
            sprintf(time_string, "%02u:%02u:%02u", hours, minutes, seconds);
            timeString = String(time_string);
            //Serial.println(timeString);
        }
        strncpy(gTimeString, timeString.c_str(), timeString.length() + 1);
    }
}

#define MAX_LINE_LENGTH 128

bool getTag(char* currentLine, char* tag) {
    if (strstr(currentLine, "StartWater") != NULL) {
        strncpy(tag, "StartWater", MAX_LINE_LENGTH);
    } else if (strstr(currentLine, "StopWater") != NULL) {
        strncpy(tag, "StopWater", MAX_LINE_LENGTH);
    } else if (strstr(currentLine, "Refresh") != NULL) {
        strncpy(tag, "Refresh", MAX_LINE_LENGTH);
    } else {
        return false;
    }
    return true;
}
DynamicJsonDocument doc(1024);
char tag[MAX_LINE_LENGTH] = "";
char currentLine[MAX_LINE_LENGTH] = "";
sTotalState totalState;
void cWIFIInterface::checkWIFI(sSoilSensorData* soilSensorData)
{
    static bool startWaterReceived = false;
    static bool startWaterReceivedLast = false;
    bool capturedTag = false;

    WiFiClient client = server.available();  // Check for incoming client requests
    if (client) {  // If a client has connected
        //Serial.println("New client connected");

        bool authenticated = true;

        while (client.connected() && client.available()) {  // While the client is connected and there's data to read
            char c = client.read();  // Read a byte
            if (c == '\n') 
            {
                // If the byte is a newline character
                // If the current line is blank, you got two newline characters in a row.
                // That's the end of the client's HTTP request:
                if (strlen(currentLine) == 0) 
                {
                    break;
                } 
                else 
                {  // If you got a newline, then clear currentLine
                    if( getTag(currentLine, tag) )
                    {
                        capturedTag = true;
                        break;
                    }
                    memset(currentLine, 0, MAX_LINE_LENGTH);
                }
            } else if (c != '\r' && strlen(currentLine) < (MAX_LINE_LENGTH - 1)) {  
                strncat(currentLine, &c, 1);
            }
        }

        if (authenticated) 
        {
            Serial.println(tag);
            if (strcmp(tag, "StartWater") == 0) 
            {
                startWaterReceived = true;
                //Serial.println("StartWater tag received.");
            }
            else if (strcmp(tag, "StopWater") == 0) 
            {
                startWaterReceived = false;
                //Serial.println("StopWater tag received.");
            }
            else if (strcmp(tag, "Refresh") == 0) 
            {
                //Serial.println("Refresh tag received.");
            }
            totalState.soilSensorData = *soilSensorData;
            totalState.wateringTimeStart = gWateringTimeStart;
            totalState.wateringDuration = gWateringDuration;
            totalState.watering = gWatering;
            // Create a JSON document

            doc["Date"] = logger.getExcelFormattedDate();
            doc["Time"] = logger.getExcelFormattedTime();
            doc["OAT"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
            doc["OAH"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
            doc["BP"].set(round(totalState.soilSensorData.baroPressure * 10.0) / 10.0);
            doc["SM"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
            doc["ST"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
            doc["SEC"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
            doc["SPH"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
            doc["WATERING"] = totalState.watering;
            float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
            if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
                wateringTimeRemaining = 0;
            }
            doc["WATERINGTIMEREMAINING"] = wateringTimeRemaining;                // Serialize JSON document to String
            String jsonString;
            serializeJson(doc, jsonString);

            // Respond to the client
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println();
            client.println(jsonString);
            
        }

        client.stop();  // Close the connection
        Serial.println("Client disconnected");

        if(startWaterReceived && !startWaterReceivedLast && !gWatering)
        {
            //Serial.println("StartWater tag received, starting watering.");
            gWatering = true;
            gWateringDuration = 60 * 10; // 10 minutes
            gWateringTimeStart = logger.getUnixTime();
        }
        else if(!startWaterReceived && startWaterReceivedLast)
        {
            //Serial.println("StopWater tag received, stopping watering.");
            gWatering = false;
        }
        startWaterReceivedLast = startWaterReceived;
    }  
}