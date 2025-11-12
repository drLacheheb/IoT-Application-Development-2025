#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

WebServer server(80);
Servo doorServo;

// Door states
enum DoorState
{
  CLOSED,
  OPENING,
  OPEN,
  CLOSING
};

DoorState currentState = CLOSED;
const int SERVO_PIN = 13;
const int LED_PIN = 2;

void setup_wifi()
{
  Serial.println("Connecting to WiFi...");
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

String getStateString()
{
  switch (currentState)
  {
  case CLOSED:
    return "CLOSED";
  case OPENING:
    return "OPENING";
  case OPEN:
    return "OPEN";
  case CLOSING:
    return "CLOSING";
  default:
    return "UNKNOWN";
  }
}

void openDoor()
{
  if (currentState == CLOSED || currentState == CLOSING)
  {
    Serial.println("Opening door...");
    currentState = OPENING;
    digitalWrite(LED_PIN, HIGH);

    for (int pos = 0; pos <= 90; pos++)
    {
      doorServo.write(pos);
      delay(15);
    }

    currentState = OPEN;
    Serial.println("Door is now OPEN");
  }
}

void closeDoor()
{
  if (currentState == OPEN || currentState == OPENING)
  {
    Serial.println("Closing door...");
    currentState = CLOSING;
    digitalWrite(LED_PIN, LOW);

    for (int pos = 90; pos >= 0; pos--)
    {
      doorServo.write(pos);
      delay(15);
    }

    currentState = CLOSED;
    Serial.println("Door is now CLOSED");
  }
}

void handleRoot()
{
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Garage Door Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html += "h1 { color: #333; }";
  html += ".status { font-size: 24px; margin: 20px; padding: 20px; ";
  html += "background-color: #f0f0f0; border-radius: 10px; }";
  html += "button { font-size: 20px; padding: 15px 30px; margin: 10px; ";
  html += "border: none; border-radius: 5px; cursor: pointer; }";
  html += ".open-btn { background-color: #4CAF50; color: white; }";
  html += ".close-btn { background-color: #f44336; color: white; }";
  html += "button:hover { opacity: 0.8; }";
  html += "</style></head><body>";
  html += "<h1>Garage Door Control</h1>";
  html += "<div class='status'>Door Status: <strong>" + getStateString() + "</strong></div>";
  html += "<button class='open-btn' onclick='sendCommand(\"open\")'>Open Door</button>";
  html += "<button class='close-btn' onclick='sendCommand(\"close\")'>Close Door</button>";
  html += "<script>";
  html += "function sendCommand(cmd) {";
  html += "  fetch('/' + cmd, { method: 'POST' })";
  html += "    .then(() => { setTimeout(() => location.reload(), 500); });";
  html += "}";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleOpen()
{
  Serial.println("Received OPEN command");
  openDoor();
  server.send(200, "text/plain", "Door opening...");
}

void handleClose()
{
  Serial.println("Received CLOSE command");
  closeDoor();
  server.send(200, "text/plain", "Door closing...");
}

void handleStatus()
{
  String json = "{\"state\":\"" + getStateString() + "\"}";
  server.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  doorServo.attach(SERVO_PIN);
  doorServo.write(0);

  setup_wifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/open", HTTP_POST, handleOpen);
  server.on("/close", HTTP_POST, handleClose);
  server.on("/status", HTTP_GET, handleStatus);

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Access the garage door control at: http://");
  Serial.println(WiFi.localIP());
}

void loop()
{
  server.handleClient();
}
