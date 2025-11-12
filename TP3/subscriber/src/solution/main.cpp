#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// CONFIGURE THESE BEFORE UPLOADING
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
const char *serverURL = "http://192.168.1.10:5000";

const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

const unsigned long POLL_INTERVAL = 2000;
unsigned long lastPollTime = 0;

String currentState = "off";

void setup_wifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        if (millis() - start > 20000)
            break;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nWiFi connection failed. Check credentials.");
    }
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

void applyState(const String &state)
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
    else
    {
        Serial.print("Unknown state: ");
        Serial.println(state);
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

    HTTPClient http;
    String url = String(serverURL) + "/command";

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        String response = http.getString();

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error)
        {
            String newState = doc["state"].as<String>();
            String timestamp = doc["timestamp"].as<String>();
            (void)timestamp; // not used but available

            if (newState != currentState)
            {
                Serial.print("New command received: ");
                Serial.println(newState);
                applyState(newState);
            }
        }
        else
        {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
        }
    }
    else
    {
        Serial.print("Error polling server. Code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void setup()
{
    Serial.begin(115200);
    delay(100);

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
    unsigned long now = millis();
    if (now - lastPollTime >= POLL_INTERVAL)
    {
        pollServer();
        lastPollTime = now;
    }

    delay(10);
}
