#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

void setup_wifi()
{
  Serial.println("Configuring static IP...");

  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("Static IP configuration failed!");
  }

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

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  allLightsOff();

  setup_wifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/red", HTTP_POST, handleRed);
  server.on("/yellow", HTTP_POST, handleYellow);
  server.on("/green", HTTP_POST, handleGreen);
  server.on("/off", HTTP_POST, handleOff);

  server.begin();
  Serial.println("Traffic Light HTTP server started");
}

void loop()
{
  server.handleClient();
}
