#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// TODO: Replace with your WiFi credentials
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

    // TODO: Move servo from 0 to 90 degrees (closed to open)
    // Hint: Use a for loop with doorServo.write() and delay()

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

    // TODO: Move servo from 90 to 0 degrees (open to closed)
    // Hint: Use a for loop with doorServo.write() and delay()

    currentState = CLOSED;
    Serial.println("Door is now CLOSED");
  }
}

void handleRoot()
{
  // TODO: Create HTML page with door controls
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Garage Door Control</title>";
  // TODO: Add CSS styling for buttons and status display
  html += "</head><body>";
  html += "<h1>Garage Door Control</h1>";
  html += "<div>Door Status: " + getStateString() + "</div>";
  // TODO: Add control buttons that send POST requests
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleOpen()
{
  // TODO: Handle door open command
  Serial.println("Received OPEN command");
  openDoor();
  server.send(200, "text/plain", "Door opening...");
}

void handleClose()
{
  // TODO: Handle door close command
  Serial.println("Received CLOSE command");
  closeDoor();
  server.send(200, "text/plain", "Door closing...");
}

void handleStatus()
{
  // TODO: Return current door state as JSON
  String json = "{\"state\":\"" + getStateString() + "\"}";
  server.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize servo
  doorServo.attach(SERVO_PIN);
  doorServo.write(0); // Start with door closed

  // Connect to WiFi
  setup_wifi();

  // TODO: Configure HTTP server routes
  // Hint: Use server.on() for each endpoint

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Access the garage door control at: http://");
  Serial.println(WiFi.localIP());
}

void loop()
{
  // TODO: Handle incoming HTTP requests
  // Hint: Call server.handleClient()
}
