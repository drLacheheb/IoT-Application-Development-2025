#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// TODO: Configure your WiFi credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// TODO: Set the traffic light ESP32's IP address
const char *trafficLightIP = "http://192.168.1.100";

WebServer server(80);
String lastCommand = "None";

void setup_wifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

bool sendCommand(String command)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return false;
    }

    // TODO: Create HTTPClient and send POST request
    // Hint: Use http.begin(), http.POST(), and check response code

    return false;
}

void handleRoot()
{
    // TODO: Create HTML page with traffic light control buttons
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Traffic Light Controller</title>";
    // Add styling and buttons for Red, Yellow, Green, Off
    html += "</head><body>";
    html += "<h1>Traffic Light Controller</h1>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleSend()
{
    // TODO: Extract command from query parameter and send to traffic light
    if (server.hasArg("cmd"))
    {
        String cmd = server.arg("cmd");
        // Send command and update lastCommand if successful
    }
    else
    {
        server.send(400, "text/plain", "Missing command parameter");
    }
}

void setup()
{
    Serial.begin(115200);

    setup_wifi();

    // TODO: Configure server routes

    server.begin();
    Serial.println("Controller HTTP server started");
    Serial.print("Access the controller at: http://");
    Serial.println(WiFi.localIP());
}

void loop()
{
    // TODO: Handle incoming HTTP requests
}
