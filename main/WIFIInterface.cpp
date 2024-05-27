#include "WIFIInterface.h"



cWIFIInterface::cWIFIInterface() : timeClient(ntpUDP, "pool.ntp.org", -25200), server(80) 
{
}

void cWIFIInterface::runWIFI(sSoilSensorData* soilSensorData, time_t epochTime)
{
    static int state = 0;
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
            static int timeOut = millis();
            static int retryCount = 0;
            
            if( millis() - timeOut > 250)
            {
                if( WiFi.status() == WL_CONNECTED )
                {
                    timeOut = millis();
                    read_dropServer();
                    update_dropServer(soilSensorData, epochTime);
                    checkWIFI(soilSensorData, epochTime);
                    retryCount = 0;
                }
                else
                {
                    if(retryCount++ > 100)
                    {
                        Serial.println("WiFi connection lost, retrying...");
                        state = 0;
                        wifiConnectionFailed = true;
                    }
                }
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

bool cWIFIInterface::CheckNtpTime(unsigned long *epochTime)
{
    static unsigned long startTime = millis();
    static unsigned long lastNtpTime = 0;


    if (millis() - startTime > 60000 || lastNtpTime == 0) // Check NTP time every minute or if it's the first run
    {
        Serial.println("Checking NTP time...");
        if (WiFi.status() == WL_CONNECTED && timeClient.update())
        {
            Serial.println("NTP time updated.");
            *epochTime = timeClient.getEpochTime();
            lastNtpTime = *epochTime; // Update lastNtpTime with the new NTP time
            startTime = millis(); // Update startTime with the current millis
            return false;
        } 
        else 
        {
            Serial.println("NTP time update failed.");
        }
    } 

    // If NTP update failed or it hasn't been a minute since the last update, calculate the time using millis
    unsigned long elapsedSeconds = (millis() - startTime) / 1000;
    *epochTime = lastNtpTime + elapsedSeconds;

    return true;
}

#define MAX_LINE_LENGTH 128

bool getTag(const std::string& currentLine, std::string& tag) {
    if (currentLine.find("StartWater") != std::string::npos) {
        tag = "StartWater";
    } else if (currentLine.find("StopWater") != std::string::npos) {
        tag = "StopWater";
    } else if (currentLine.find("Refresh") != std::string::npos) {
        tag = "Refresh";
    } else {
        return false;
    }
    return true;
}
DynamicJsonDocument doc(512);
char jsonString[512];
std::string tag;
std::string currentLine;
sTotalState totalState;

// app.get('/manualWaterStatus', (req, res) => {
//   fs.readFile(manualWaterOverrideFilePath, 'utf8', (err, data) => {
//     if (err) {
//       console.error('Error reading from manualWaterOverride.csv', err);
//       res.status(200).send('0');
//     } else {
//       console.log('Manual water status:', data);
//       if (data.trim() === 'true') {
//         res.status(200).send('1');
//       } else {
//         res.status(200).send('0');
//       }
//     }
//   });
// });


// app.get('/manualWaterStatus', (req, res) => {
//   fs.readFile(manualWaterOverrideFilePath, 'utf8', (err, data) => {
//     if (err) {
//       console.error('Error reading from manualWaterOverride.csv', err);
//       res.status(200).send('-1');
//     } else {
//       console.log('Manual water status:', data);
//       if (data.trim() === 'true') {
//         res.status(200).send('1');
//       } else {
//         res.status(200).send('0');
//       }
//     }
//   });
// });

void cWIFIInterface::read_dropServer()
{
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 10000) 
    {
        return;
    }
    lastUpdate = millis();

    WiFiClient client;
    const char* server = "64.23.202.34";  // Replace with your server's IP address
    const int port = 3000;  // Replace with your server's port

    bool manualWateringRequest = gWatering;

    Serial.println("read_dropServer...");

    if (client.connect(server, port)) {
        client.println("GET /manualWaterStatus HTTP/1.1");
        client.println("Host: " + String(server));
        client.println("Connection: close");
        client.println();

        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                if (line == "1") {
                    manualWateringRequest = true;
                } else if (line == "0") {
                    manualWateringRequest = false;
                }
                Serial.print("manualWateringRequest: ");
                Serial.println(line);
            }
        }
        client.stop();
    } else {
        Serial.println("connection failed");
        client.stop();
    }

    bool autoWateringRequest = gAutoWateringEnabled;
    if (client.connect(server, port)) {
        client.println("GET /autoWaterStatus HTTP/1.1");
        client.println("Host: " + String(server));
        client.println("Connection: close");
        client.println();

        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                if (line == "1") {
                    autoWateringRequest = true;
                } else if (line == "0") {
                    autoWateringRequest = false;
                }
                Serial.print("autoWateringRequest: ");
                Serial.println(line);
            }
        }
        client.stop();
    } else {
        Serial.println("connection failed");
        client.stop();
    }

    setManualWaterStatus(manualWateringRequest);
    setAutolWaterStatus(autoWateringRequest);

}

void cWIFIInterface::update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 10000) 
    {
        return;
    }
    lastUpdate = millis();
    totalState.soilSensorData = *soilSensorData;
    totalState.wateringTimeStart = gWateringTimeStart;
    totalState.wateringDuration = gWateringDuration;
    totalState.watering = gWatering;
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

    // snprintf(jsonData, sizeof(jsonData),
    //     "{"
    // *    "\"DateStamp\": \"2023-05-25\","
    // *    "\"TimeStamp\": \"12:34:56\","
    // *    "\"Epoch\": %lu,"
    // *    "\"OutsideAirTemp\": 22.5,"
    // *    "\"OutsideHumidity\": 45.2,"
    // *    "\"OutsideBaro\": 1013.1,"
    // *    "\"SoilTemperature\": 20.3,"
    // *    "\"SoilElectricalConductivity\": 1.2,"
    // *    "\"SoilHumidity\": 30.1,"
    // *    "\"SoilPh\": 6.5,"
    // *    "\"Watering\": true,"
    // *    "\"TimeRemaining\": 120,"
    // *    "\"WifiError\": false,"
    // *    "\"SDError\": false,"
    // *    "\"RTCFailed\": false"
    //     "}", epoch);

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
    float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
    if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
        wateringTimeRemaining = 0;
    }
    doc["TimeRemaining"] = wateringTimeRemaining;
    doc["WifiError"] = wifiConnectionFailed;
    doc["SDError"] = !SD.exists(FileName);
    doc["RTCFailed"] = rtcFailed;

    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("update_dropServer: WiFi not connected. Attempting to reconnect...");

    }

    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    Serial.println(strlen(jsonString));
    //char remoteServer[] = "192.168.1.31"; // address of your server
    char remoteServer[] = "64.23.202.34"; // address of your server
    WiFiClient client;
    if (client.connect(remoteServer, 3000)) {
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
    else {
        // if you couldn't make a connection:
        Serial.println("connection failed");
        client.stop();  // Ensure the client is stopped
    }
}
/*
{"DateStamp":"2024-05-26","TimeStamp":"15:07:17","Epoch":1716736037,"OutsideAirTemp":72.9,"OutsideHumidity":46.1,"OutsideBaro":28.6,"SoilTemperature":67.8,"SoilElectricalConductivity":183,"SoilHumidity":28.5,"SoilPh":8.3,"Watering":false,"TimeRemaining":0,"WifiError":false,"SDError":false,"RTCFailed":false}
308
con
*/
void cWIFIInterface::checkWIFI(sSoilSensorData* soilSensorData, time_t epochTime)
{
    static bool startWaterReceived = false;
    bool capturedTag = false;
    currentLine.clear();
    tag.clear();
    WiFiClient client;
    client = server.available();  // Check for incoming client requests
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
                if (currentLine.length() == 0) 
                {
                    break;
                } 
                else 
                {  // If you got a newline, then clear currentLine
                    //Serial.println(currentLine.c_str());
                    if( getTag(currentLine.c_str(), tag) )
                    {
                        capturedTag = true;
                        break;
                    }
                    currentLine.clear();
                }
            } else if (c != '\r' && currentLine.length() < (MAX_LINE_LENGTH - 1)) {  
                currentLine += c;
            }
        }

        if (authenticated) 
        {
            Serial.println(tag.c_str());
            if (tag == "StartWater") 
            {
                startWaterReceived = true;
                //Serial.println("StartWater tag received.");
            }
            else if (tag == "StopWater") 
            {
                startWaterReceived = false;
                //Serial.println("StopWater tag received.");
            }
            else if (tag == "Refresh") 
            {
                //Serial.println("Refresh tag received.");
            }
            totalState.soilSensorData = *soilSensorData;
            totalState.wateringTimeStart = gWateringTimeStart;
            totalState.wateringDuration = gWateringDuration;
            totalState.watering = gWatering;
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
            doc["Time"] = timeBuffer;
            doc["Date"] = dateBuffer;
            doc["OAT"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
            doc["OAH"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
            doc["BP"].set(round(totalState.soilSensorData.baroPressure * 100.0) / 100.0);
            doc["SM"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
            doc["ST"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
            doc["SEC"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
            doc["SPH"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
            doc["WATERING"] = totalState.watering;
            float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
            if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
                wateringTimeRemaining = 0;
            }
            doc["WATERINGTIMEREMAINING"] = wateringTimeRemaining;      

            //these are set to 512. you want to verify that this is enough space for your json object if you add more fields.
            serializeJson(doc, jsonString);

            // Respond to the client
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println();
            client.println(jsonString);
            
        }

        client.stop();  // Close the connection
        Serial.println("Client disconnected");

        // if(startWaterReceived && !startWaterReceivedLast && !gWatering)
        // {
        //     //Serial.println("StartWater tag received, starting watering.");
        //     gWatering = true;
        //     gWateringDuration = 60 * 10; // 10 minutes
        //     gWateringTimeStart = logger.getUnixTime();
        // }
        // else if(!startWaterReceived && startWaterReceivedLast)
        // {
        //     //Serial.println("StopWater tag received, stopping watering.");
        //     gWatering = false;
        // }
        // startWaterReceivedLast = startWaterReceived;

        setManualWaterStatus(startWaterReceived);
    }  
}

void cWIFIInterface::setManualWaterStatus(bool request)
{
    bool startWaterReceived = request;
    static bool startWaterReceivedLast = false;

    if(startWaterReceived && !startWaterReceivedLast && !gWatering)
    {
        gWatering = true;
        gWateringDuration = 60 * 10; // 10 minutes
        gWateringTimeStart = logger.getUnixTime();
    }
    else if(!startWaterReceived && startWaterReceivedLast)
    {
        gWatering = false;
    }
    startWaterReceivedLast = startWaterReceived;
}

void cWIFIInterface::setAutolWaterStatus(bool request)
{
    bool startWaterReceived = request;
    static bool startWaterReceivedLast = false;

    if(startWaterReceived && !startWaterReceivedLast && !gWatering)
    {
        gAutoWateringEnabled = true;
    }
    else if(!startWaterReceived && startWaterReceivedLast)
    {
        gAutoWateringEnabled = false;
    }
    startWaterReceivedLast = startWaterReceived;
}
