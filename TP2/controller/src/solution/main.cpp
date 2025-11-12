#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

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

  HTTPClient http;
  String url = String(trafficLightIP) + command;

  Serial.print("Sending command to: ");
  Serial.println(url);

  http.begin(url);
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.POST("");

  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(response);
    http.end();
    return true;
  }
  else
  {
    Serial.print("Error sending command. Code: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void handleRoot()
{
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Traffic Light Controller</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html += "h1 { color: #333; }";
  html += ".button { font-size: 20px; padding: 20px 40px; margin: 10px; ";
  html += "border: none; border-radius: 5px; color: white; cursor: pointer; }";
  html += ".red-btn { background-color: #ff0000; }";
  html += ".yellow-btn { background-color: #ffcc00; color: black; }";
  html += ".green-btn { background-color: #00ff00; color: black; }";
  html += ".off-btn { background-color: #666666; }";
  html += ".status { margin: 20px; padding: 15px; background-color: #f0f0f0; }";
  html += "</style></head><body>";
  html += "<h1>🚦 Traffic Light Controller</h1>";
  html += "<div class='status'>Last Command: <strong>" + lastCommand + "</strong></div>";
  html += "<button class='button red-btn' onclick='sendCmd(\"/red\")'>RED</button><br>";
  html += "<button class='button yellow-btn' onclick='sendCmd(\"/yellow\")'>YELLOW</button><br>";
  html += "<button class='button green-btn' onclick='sendCmd(\"/green\")'>GREEN</button><br>";
  html += "<button class='button off-btn' onclick='sendCmd(\"/off\")'>OFF</button>";
  html += "<script>";
  html += "function sendCmd(cmd) {";
  html += "  fetch('/send?cmd=' + cmd)";
  html += "    .then(response => response.text())";
  html += "    .then(() => location.reload());";
  html += "}";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleSend()
{
  if (server.hasArg("cmd"))
  {
    String cmd = server.arg("cmd");
    bool success = sendCommand(cmd);

    if (success)
    {
      lastCommand = cmd;
      server.send(200, "text/plain", "Command sent successfully");
    }
    else
    {
      server.send(500, "text/plain", "Failed to send command");
    }
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

  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_GET, handleSend);

  server.begin();
  Serial.println("Controller HTTP server started");
  Serial.print("Access the controller at: http://");
  Serial.println(WiFi.localIP());
}

void loop()
{
  server.handleClient();
}
