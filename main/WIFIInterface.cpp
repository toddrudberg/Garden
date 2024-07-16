#include "WIFIInterface.h"



cWIFIInterface::cWIFIInterface() : timeClient(ntpUDP, "pool.ntp.org", -25200), server(80) 
{
}

void cWIFIInterface::runWIFI(sSoilSensorData* soilSensorData, time_t epochTime)
{
    static int state = 0;
    static bool wifiConnectionFailed = false;
    switch(state)
    {
        case 0:
        {
            static int timeOut = millis();

            if( !wifiConnectionFailed && setupWIFI())
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
                wifiConnectionFailed = true;
                if(millis() - timeOut > 2000)
                {
                    Serial.println("WIFI setup failed, retrying...");
                    timeOut = millis();
                    wifiConnectionFailed = false;
                }
            }
            break;
        }
        case 1:
        {
            static unsigned long lastAttemptTime = millis();
            static unsigned long printToScreen = 0;
            const unsigned long retryInterval = 5000; // Retry every 5 seconds
            const unsigned long maxServerFails = 30; // Maximum number of server fails before resetting WiFi, about 5 minutes

            if (WiFi.status() == WL_CONNECTED) 
            {
                bool serverConnection = manageDropServer(soilSensorData, epochTime);

                if (millis() - printToScreen > 10000) 
                {
                    Serial.print("serverConnection: ");
                    Serial.print(serverConnection);
                    Serial.print(" RemoteServerFails: ");
                    Serial.println(gremoteServerFails);
                    printToScreen = millis();
                }

                if (!serverConnection) 
                {
                    gremoteServerFails++;
                }

                if (gremoteServerFails > maxServerFails && !gAutoWateringCycleOn)
                {
                    Serial.println("Too many server fails, resetting WiFi...");
                    WiFi.disconnect();
                    gremoteServerFails = 0;
                    state = 0; // Go back to state 0 to reconnect
                    NVIC_SystemReset();
                }
            } 
            else 
            {
                if (millis() - lastAttemptTime > retryInterval) 
                {
                    Serial.println("WiFi connection lost, retrying...");
                    state = 0; // Go back to state 0 to reconnect
                    lastAttemptTime = millis(); // Update the last attempt time
                }
            }
            break;
        }
    }
}

bool cWIFIInterface::manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    const unsigned long serverRest = 10000; //general rest time between server requests
    const unsigned long serverUpdateLogDelay = 5000; //if a state change, we wait this long before sending the update.  must be smaller than serverRest
    const unsigned long serverUpdateLogInterval = 60000; //generally, we log every 60 seconds
    static unsigned long lastServerRequestTime = 0;
    static unsigned long lastLogtime = 0;
    static int state = 0;
    bool serverConnection = true;
    bool manualWateringRequest = gManualWateringOn;
    bool autoWateringRequest = gAutoWateringEnabled;
    bool refreshRequest = false;

    static bool refreshRequestLast = false;
    static bool autoWateringRequestlast = false;
    static bool manualWateringRequestlast = false;

    bool updateServer = false;
    if (millis() - lastServerRequestTime > serverRest) 
    {
        switch (state) 
        {
            case 0:
                serverConnection = read_dropServer(&autoWateringRequest, &manualWateringRequest, &refreshRequest);
                Serial.println(" ");
                Serial.println("Manual Watering Request: " + String(manualWateringRequest ? "True" : "False"));
                Serial.println("Auto Watering Request: " + String(autoWateringRequest ? "True" : "False"));
                Serial.println("Refresh Request: " + String(refreshRequest ? "True" : "False"));
                Serial.println(" ");
                if(serverConnection)
                {
                    setAutolWaterStatus(autoWateringRequest);
                    setManualWaterStatus(manualWateringRequest, epochTime);
                    updateServer  =  autoWateringRequest != autoWateringRequestlast || 
                                        manualWateringRequest != manualWateringRequestlast|| 
                                        refreshRequest != refreshRequestLast ;
                    if(millis() - lastLogtime > serverUpdateLogInterval || updateServer)
                    {
                        autoWateringRequestlast = autoWateringRequest;
                        manualWateringRequestlast = manualWateringRequest;
                        refreshRequestLast = refreshRequest;
                        state++;
                    }
                }
                lastServerRequestTime = updateServer ? millis() + serverRest - serverUpdateLogDelay : millis();
                //lastServerRequestTime = millis();  //this is more conservative. we will always wait serverRest time before trying again
                break;
            case 1:
                serverConnection = update_dropServer(soilSensorData, epochTime);                
                state = 0;
                lastLogtime = millis();
                lastServerRequestTime = millis();
                break;
            default:
                state = 0;
                lastServerRequestTime = millis();
                break;
        }

    }
    return serverConnection;
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

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        if (millis() - startTime > 5000) // 5 second timeout
        {
            Serial.println("Failed to connect to WiFi");
            return false;
        }
        delay(100);
    }

    Serial.println("Connected to WiFi");
    return true;
}

bool cWIFIInterface::CheckNtpTime(unsigned long *epochTime)
{
    static unsigned long startTime = millis();
    static unsigned long lastNtpTime = 0;
    unsigned long currentTime = millis();
    unsigned long updateIntervalMinutes = 60;
    unsigned long updateInterval = updateIntervalMinutes  * 60 * 1000; 
    bool isTimeForUpdate = (currentTime - startTime > updateInterval) || (lastNtpTime == 0);

    if (!isTimeForUpdate) 
    {
        *epochTime = lastNtpTime + ((currentTime - startTime) / 1000);
        return true;
    }

    if (WiFi.status() != WL_CONNECTED || !timeClient.update()) 
    {
        Serial.println("NTP time update failed.");
        return false;
    }

    unsigned long currentEpochTime = timeClient.getEpochTime();
    DateTime now = DateTime(currentEpochTime);
    if (now.year() < 2024 || now.year() > 2025) 
    {
        Serial.println("Invalid NTP time.");
        return false;
    }

    Serial.println("NTP time updated.");
    *epochTime = timeClient.getEpochTime();
    lastNtpTime = *epochTime;
    startTime = currentTime;

    return true;
}


DynamicJsonDocument doc(512);
char jsonString[512];
sTotalState totalState;

bool cWIFIInterface::read_dropServer(bool* autoWateringRequest, bool* manualWaterOverrideRequest, bool* aRefreshRequest)
{
    bool connectionSolid = true;
    WiFiClient client;

    if (client.connect(remoteServer, remoteServerPort)) {
        client.println("GET /status HTTP/1.1");
        client.println("Host: " + String(remoteServer));
        client.println("Connection: close");
        client.println();
        unsigned long startTimems = millis();
        while (client.connected() && millis() - startTimems < 5000)
        {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                if (line.startsWith("{\"manualWaterOverride\":")) 
                {
                    DynamicJsonDocument doc(1024);
                    deserializeJson(doc, line);

                    *autoWateringRequest = doc["autoWaterStatus"];
                    *manualWaterOverrideRequest = doc["manualWaterOverride"];
                    *aRefreshRequest = doc["gRefreshRequest"];
                    break;
                }
            }
        }
        Serial.print("autoWateringRequest: ");
        Serial.println(*autoWateringRequest);
        Serial.print("manualWaterOverrideRequest: ");
        Serial.println(*manualWaterOverrideRequest);
        Serial.print("gRefreshRequest: ");
        Serial.println(*aRefreshRequest);
        client.stop();
    } 
    else 
    {
        connectionSolid = false;
        Serial.println("connection failed in read_dropServer");
        client.stop();
    }
    return connectionSolid;
}

bool cWIFIInterface::update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    Serial.println("update_dropServer...");
    bool connectionSolid = true;
    totalState.soilSensorData = *soilSensorData;
    totalState.wateringTimeStart = gWateringTimeStart;
    totalState.wateringDuration = gWateringDuration;
    totalState.watering = gManualWateringOn || gAutoWateringCycleOn;
    totalState.autoWaterCycleActive = gAutoWateringEnabled;
    
    // Create a JSON document
    
    // Convert the epoch time to a struct tm
    struct tm *timeStruct = localtime(&epochTime);
    // Create a buffer to hold the time string
    char dateBuffer[11];
    char timeBuffer[9];
    // Format the struct tm as a date string
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeStruct);

    // Format the struct tm as a time string
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeStruct);

    // Add the time string to the JSON document
    doc["DateStamp"] = dateBuffer;
    doc["TimeStamp"] = timeBuffer;
    doc["Epoch"] = epochTime;
    doc["OutsideAirTemp"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
    doc["OutsideHumidity"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
    doc["OutsideBaro"].set(round(totalState.soilSensorData.baroPressure * 100.0) / 100.0);
    doc["SoilTemperature"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
    doc["SoilElectricalConductivity"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
    doc["SoilHumidity"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
    doc["SoilPh"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
    doc["Watering"] = totalState.watering;
    
#ifndef DEBUGER
    unsigned long ulEpochTime = USERTC ? logger.getUnixTime() : static_cast<unsigned long>(epochTime);
    float wateringTimeRemaining = (totalState.wateringDuration - (ulEpochTime - totalState.wateringTimeStart)) / 60.0;
#else
    float wateringTimeRemaining = (totalState.wateringDuration - (epochTime - totalState.wateringTimeStart)) / 60.0;
#endif
    if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
        wateringTimeRemaining = 0;
    }
    doc["autoWaterCycleEnabled"] = totalState.autoWaterCycleActive;
    doc["TimeRemaining"] = wateringTimeRemaining;
    doc["WifiError"] = WiFi.status() != WL_CONNECTED;
#ifndef DEBUGER
    doc["SDError"] = !SD.exists(FileName);
#else
    doc["SDError"] = false;
#endif
    doc["RTCFailed"] = rtcFailed;
    doc["AvgTempPrevDay"] = soilSensorData->avgOATPreviousDay;

    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("update_dropServer: WiFi not connected. Attempting to reconnect...");
    }

    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    Serial.println(strlen(jsonString));
    WiFiClient client;
    if (client.connect(remoteServer, remoteServerPort)) {
        // send the HTTP POST request:
        client.println("POST /log HTTP/1.1");
        client.println("Host: " + String(remoteServer));
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        // calculate the length of the JSON data
        client.print("Content-Length: ");
        client.println(strlen(jsonString));
        client.println();
        // send the JSON data:
        client.println(jsonString);
        Serial.println("Data Sent");
        client.stop();  // Ensure the client is stopped
    } 
    else 
    {
        // if you couldn't make a connection:
        connectionSolid = false;
        Serial.println("connection failed in update_dropServer");
        client.stop();  // Ensure the client is stopped
    }
    return connectionSolid;
}


void cWIFIInterface::setManualWaterStatus(bool request, time_t epochTime)
{
    bool startWaterReceived = request;
    static bool startWaterReceivedLast = false;

    if(startWaterReceived && !gManualWateringOn && !startWaterReceivedLast)
    {
        gManualWateringOn = true;
        gWateringDuration = 60 * 10; // 10 minutes
        unsigned long ulEpochTime = USERTC ? logger.getUnixTime() : static_cast<unsigned long>(epochTime);
        gWateringTimeStart = ulEpochTime;
    }
    else if(!startWaterReceived)
    {
        gManualWateringOn = false;
    }
    startWaterReceivedLast = startWaterReceived;
}

void cWIFIInterface::setAutolWaterStatus(bool request)
{
    // we may make more logic here...so we will keep this function
    bool startWaterReceived = request;
    gAutoWateringEnabled = startWaterReceived;
}

//#define MAX_LINE_LENGTH 128

// bool getTag(const std::string& currentLine, std::string& tag) {
//     if (currentLine.find("StartWater") != std::string::npos) {
//         tag = "StartWater";
//     } else if (currentLine.find("StopWater") != std::string::npos) {
//         tag = "StopWater";
//     } else if (currentLine.find("Refresh") != std::string::npos) {
//         tag = "Refresh";
//     } else {
//         return false;
//     }
//     return true;
// }


// bool cWIFIInterface::read_dropServer(int requestType)
// {
//     bool connectionSolid = true;



//     Serial.print("read_dropServer...");
//     Serial.println(requestType);
//     if( requestType == 1)
//     {
//         WiFiClient client;
//         bool manualWateringRequest = gManualWateringOn;
//         if (client.connect(remoteServer, remoteServerPort)) 
//         {
//             client.println("GET /manualWaterStatus HTTP/1.1");
//             client.println("Host: " + String(remoteServer));
//             client.println("Connection: close");
//             client.println();
//             unsigned long startTimems = millis();
//             while (client.connected() && millis() - startTimems < 1000)
//             {
//                 if (client.available()) 
//                 {
//                     String line = client.readStringUntil('\n');
//                     if (line == "1") 
//                     {
//                         manualWateringRequest = true;
//                         break;
//                     } 
//                     else if (line == "0") 
//                     {
//                         manualWateringRequest = false;
//                         break;
//                     }
//                 }
//             }
//             Serial.print("manualWateringRequest: ");
//             Serial.println(manualWateringRequest);
//             client.stop();
//             setManualWaterStatus(manualWateringRequest);
//         } 
//         else 
//         {
//             connectionSolid = false;
//             Serial.println("connection failed in read_dropServer");
//             client.stop();
//         }
//     }

//     if( requestType == 2)
//     {
//         WiFiClient client;      
//         bool autoWateringRequest = gAutoWateringEnabled;
//         if (client.connect(remoteServer, remoteServerPort)) {
//             client.println("GET /autoWaterStatus HTTP/1.1");
//             client.println("Host: " + String(remoteServer));
//             client.println("Connection: close");
//             client.println();
//             unsigned long startTimems = millis();
//             while (client.connected() && millis() - startTimems < 1000)
//             {
//                 if (client.available()) {
//                     String line = client.readStringUntil('\n');
//                     if (line == "1") 
//                     {
//                         autoWateringRequest = true;
//                         break;
//                     } 
//                     else if (line == "0") 
//                     {
//                         autoWateringRequest = false;
//                         break;
//                     }
//                 }
//             }
//             Serial.print("autoWateringRequest: ");
//             Serial.println(autoWateringRequest);
//             client.stop();
//             setAutolWaterStatus(autoWateringRequest);
//         } 
//         else 
//         {
//             connectionSolid = false;
//             Serial.println("connection failed in read_dropServer");
//             client.stop();
//         }
//     }
//     return connectionSolid;
// }


// {"DateStamp":"2024-05-26","TimeStamp":"15:07:17","Epoch":1716736037,"OutsideAirTemp":72.9,"OutsideHumidity":46.1,"OutsideBaro":28.6,"SoilTemperature":67.8,"SoilElectricalConductivity":183,"SoilHumidity":28.5,"SoilPh":8.3,"Watering":false,"TimeRemaining":0,"WifiError":false,"SDError":false,"RTCFailed":false}
// 308
// con

// void cWIFIInterface::checkWIFI(sSoilSensorData* soilSensorData, time_t epochTime)
// {
//     static bool startWaterReceived = false;
//     bool capturedTag = false;
//     currentLine.clear();
//     tag.clear();
//     WiFiClient client;
//     client = server.available();  // Check for incoming client requests
//     if (client) {  // If a client has connected
//         //Serial.println("New client connected");

//         bool authenticated = true;

//         while (client.connected() && client.available()) {  // While the client is connected and there's data to read
//             char c = client.read();  // Read a byte
//             if (c == '\n') 
//             {
//                 // If the byte is a newline character
//                 // If the current line is blank, you got two newline characters in a row.
//                 // That's the end of the client's HTTP request:
//                 if (currentLine.length() == 0) 
//                 {
//                     break;
//                 } 
//                 else 
//                 {  // If you got a newline, then clear currentLine
//                     //Serial.println(currentLine.c_str());
//                     if( getTag(currentLine.c_str(), tag) )
//                     {
//                         capturedTag = true;
//                         break;
//                     }
//                     currentLine.clear();
//                 }
//             } else if (c != '\r' && currentLine.length() < (MAX_LINE_LENGTH - 1)) {  
//                 currentLine += c;
//             }
//         }

//         if (authenticated) 
//         {
//             Serial.println(tag.c_str());
//             if (tag == "StartWater") 
//             {
//                 startWaterReceived = true;
//                 //Serial.println("StartWater tag received.");
//             }
//             else if (tag == "StopWater") 
//             {
//                 startWaterReceived = false;
//                 //Serial.println("StopWater tag received.");
//             }
//             else if (tag == "Refresh") 
//             {
//                 //Serial.println("Refresh tag received.");
//             }
//             totalState.soilSensorData = *soilSensorData;
//             totalState.wateringTimeStart = gWateringTimeStart;
//             totalState.wateringDuration = gWateringDuration;
//             totalState.watering = gManualWateringOn || gAutoWateringCycleOn;
//             totalState.autoWaterCycleActive = gAutoWateringEnabled;
//             // Create a JSON document
            
//             // Convert the epoch time to a struct tm
//             struct tm *timeStruct = localtime(&epochTime);
//             // Create a buffer to hold the time string
//             char dateBuffer[11];
//             char timeBuffer[9];
//             // Format the struct tm as a date string
//             strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeStruct);

//             // Format the struct tm as a time string
//             strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeStruct);

//             // Add the time string to the JSON document
//             doc["Time"] = timeBuffer;
//             doc["Date"] = dateBuffer;
//             doc["OAT"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
//             doc["OAH"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
//             doc["BP"].set(round(totalState.soilSensorData.baroPressure * 100.0) / 100.0);
//             doc["SM"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
//             doc["ST"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
//             doc["SEC"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
//             doc["SPH"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
//             doc["WATERING"] = totalState.watering;
//             float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
//             if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
//                 wateringTimeRemaining = 0;
//             }
//             doc["WATERINGTIMEREMAINING"] = wateringTimeRemaining;      

//             //these are set to 512. you want to verify that this is enough space for your json object if you add more fields.
//             serializeJson(doc, jsonString);

//             // Respond to the client
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-Type: application/json");
//             client.println();
//             client.println(jsonString);
            
//         }

//         client.stop();  // Close the connection
//         Serial.println("Client disconnected");

//         setManualWaterStatus(startWaterReceived);
//     }  
// }