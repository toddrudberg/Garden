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
    
    WiFi.begin(ssid, password);
    return WiFi.status() == WL_CONNECTED;  
}

void cWIFIInterface::checkWIFI(sSoilSensorData* soilSensorData)
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

    WiFiClient client = server.available();  // Check for incoming client requests
    if (client) {  // If a client has connected
        Serial.println("New client connected");
        String currentLine = "";

        bool authenticated = true;
        if(false)
        {
            while (client.connected()) 
            {
                if (client.available()) 
                {
                char c = client.read();
                    if (c == '\n') 
                    {
                        if (currentLine.startsWith("Authorization: Basic")) {
                        String auth = currentLine.substring(21);
                        // auth now contains the Base64-encoded username:password
                        // Check if auth matches your expected username:password
                        //http://admin:1234@192.168.0.91
                        if (auth == "YWRtaW46MTIzNA==") {
                            authenticated = true;
                        }
                        authenticated = true;
                        }
                        currentLine = "";
                    } else if (c != '\r') {
                        currentLine += c;
                    }
                }
            }
        }

        if (authenticated) 
        {
            Serial.println("Client authenticated at " + timeString);
            // Respond to the client
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<html><head><title>Hello, Arduino!</title></head><body>");
            client.println("<h1>Hello, World!</h1>");
            client.println("<p id='clientTime'></p>");  // Paragraph where the client's time will be displayed
            client.println("<p id='serverTime'></p>");  // Paragraph where the server's time will be displayed
            client.println("<h1>Soil Sensor Data</h1>");
            client.println("<p>Soil Moisture: " + String(soilSensorData->soilMoisture) + "</p>");
            client.println("<p>Soil Temperature: " + String(soilSensorData->soilTemperature) + "</p>");
            client.println("<p>Soil Electrical Conductivity: " + String(soilSensorData->soilElectricalConductivity) + "</p>");
            client.println("<p>Soil pH: " + String(soilSensorData->soilPh) + "</p>");
            client.println("<script>");
            client.println("function updateTime() {");
            client.println("  var d = new Date();");  // Create a new Date object
            client.println("  var hours = d.getHours() < 10 ? '0' + d.getHours().toString() : d.getHours().toString();");
            client.println("  var minutes = d.getMinutes() < 10 ? '0' + d.getMinutes().toString() : d.getMinutes().toString();");
            client.println("  var seconds = d.getSeconds() < 10 ? '0' + d.getSeconds().toString() : d.getSeconds().toString();");
            client.println("  var timeString = hours + ':' + minutes + ':' + seconds;");  // Format the time    
            client.println("  document.getElementById('clientTime').textContent = 'Client Time: ' + timeString;");  // Update the client's time
            client.println("  document.getElementById('serverTime').textContent = 'Server Time: " + String(timeClient.getFormattedTime()) + "';");  // Update the server's time
            client.println("}");
            client.println("setInterval(updateTime, 1000);");  // Update the time every second
            client.println("updateTime();");  // Update the time immediately
            client.println("</script>");
            client.println("</body></html>");
            }
        else 
        {
            // The client is not authenticated
            // Send a 401 Unauthorized response
            client.println("HTTP/1.1 401 Unauthorized");
            client.println("WWW-Authenticate: Basic realm=\"Secure Area\"");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<html><body><h1>401 Unauthorized</h1></body></html>");
        }

        client.stop();  // Close the connection
        Serial.println("Client disconnected");
    }  
}