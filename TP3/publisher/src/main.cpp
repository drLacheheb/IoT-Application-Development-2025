#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// TODO: Configure your WiFi credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// TODO: Set your server address (replace with your computer's IP)
const char *serverURL = "http://192.168.1.10:5000";

WebServer server(80);
String lastCommand = "none";
bool lastSendSuccess = false;

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

bool sendCommandToServer(String state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return false;
    }

    // TODO: Create HTTPClient and send POST request with JSON payload
    // Hint: Use ArduinoJson to create {"state": "...", "source": "ESP32-Publisher"}

    return false;
}

void handleRoot()
{
    String statusColor = lastSendSuccess ? "green" : "red";
    String statusText = lastSendSuccess ? "Connected" : "Disconnected";

    // TODO: Create HTML page with control buttons
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Traffic Light Publisher</title>";
    html += "</head><body>";
    html += "<h1>Traffic Light Publisher</h1>";
    html += "<p>Last Command: " + lastCommand + "</p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleSend()
{
    if (server.hasArg("state"))
    {
        String state = server.arg("state");
        lastSendSuccess = sendCommandToServer(state);
        lastCommand = state;

        if (lastSendSuccess)
        {
            server.send(200, "text/plain", "Command sent to server");
        }
        else
        {
            server.send(500, "text/plain", "Failed to send command");
        }
    }
    else
    {
        server.send(400, "text/plain", "Missing state parameter");
    }
}

void setup()
{
    Serial.begin(115200);

    setup_wifi();

    // TODO: Configure server routes

    server.begin();
    Serial.println("Publisher HTTP server started");
    Serial.print("Access at: http://");
    Serial.println(WiFi.localIP());
}

void loop()
{
    server.handleClient();
}
