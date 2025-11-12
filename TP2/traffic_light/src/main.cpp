#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// TODO: Configure your WiFi credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// TODO: Set a static IP for this device
IPAddress local_IP(192, 168, 1, 100); // Traffic Light IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// LED pins
const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

void setup_wifi()
{
    Serial.println("Configuring static IP...");

    // TODO: Configure static IP
    // Hint: Use WiFi.config(local_IP, gateway, subnet)

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

void setRedLight()
{
    // TODO: Turn on RED LED, turn off others
}

void setYellowLight()
{
    // TODO: Turn on YELLOW LED, turn off others
}

void setGreenLight()
{
    // TODO: Turn on GREEN LED, turn off others
}

void allLightsOff()
{
    // TODO: Turn off all LEDs
}

void handleRed()
{
    setRedLight();
    server.send(200, "text/plain", "Red light activated");
}

void handleYellow()
{
    setYellowLight();
    server.send(200, "text/plain", "Yellow light activated");
}

void handleGreen()
{
    setGreenLight();
    server.send(200, "text/plain", "Green light activated");
}

void handleOff()
{
    allLightsOff();
    server.send(200, "text/plain", "All lights off");
}

void handleRoot()
{
    String html = "<html><body>";
    html += "<h1>Traffic Light System</h1>";
    html += "<p>Listening for commands...</p>";
    html += "<p>Endpoints: /red, /yellow, /green, /off</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup()
{
    Serial.begin(115200);

    // TODO: Initialize LED pins as OUTPUT

    allLightsOff();

    setup_wifi();

    // TODO: Configure server routes
    // Hint: server.on("/path", HTTP_METHOD, handler_function)

    server.begin();
    Serial.println("Traffic Light HTTP server started");
}

void loop()
{
    // TODO: Handle incoming HTTP requests
}
