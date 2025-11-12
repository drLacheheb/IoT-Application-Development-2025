#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// TODO: Configure your WiFi credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// TODO: Set your server address
const char *serverURL = "http://192.168.1.10:5000";

// LED pins
const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

// Polling interval (milliseconds)
const unsigned long POLL_INTERVAL = 2000; // 2 seconds
unsigned long lastPollTime = 0;

String currentState = "off";

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

void setRedLight()
{
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("RED light ON");
}

void setYellowLight()
{
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("YELLOW light ON");
}

void setGreenLight()
{
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("GREEN light ON");
}

void allLightsOff()
{
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("All lights OFF");
}

void applyState(String state)
{
    if (state == "red")
    {
        setRedLight();
    }
    else if (state == "yellow")
    {
        setYellowLight();
    }
    else if (state == "green")
    {
        setGreenLight();
    }
    else if (state == "off")
    {
        allLightsOff();
    }
    currentState = state;
}

void pollServer()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return;
    }

    // TODO: Send GET request to server /command endpoint
    // Hint: Use HTTPClient, parse JSON response, and check if state changed
}

void setup()
{
    Serial.begin(115200);

    // Initialize LED pins
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    allLightsOff();

    setup_wifi();

    Serial.println("Traffic Light Subscriber started");
    Serial.println("Polling server for commands...");
}

void loop()
{
    unsigned long currentTime = millis();

    // TODO: Poll server at regular intervals
    // Hint: Check if POLL_INTERVAL has passed since last poll

    delay(10);
}
