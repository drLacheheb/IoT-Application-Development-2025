#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// CONFIGURE THESE BEFORE UPLOADING
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
const char *serverURL = "http://192.168.1.10:5000";

WebServer server(80);
String lastCommand = "none";
bool lastSendSuccess = false;

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

bool sendCommandToServer(const String &state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return false;
    }

    HTTPClient http;
    String url = String(serverURL) + "/update";

    StaticJsonDocument<200> doc;
    doc["state"] = state;
    doc["source"] = "ESP32-Publisher";

    String jsonBody;
    serializeJson(doc, jsonBody);

    Serial.print("Sending to server: ");
    Serial.println(jsonBody);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode > 0)
    {
        String response = http.getString();
        Serial.print("Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
        http.end();
        return (httpResponseCode >= 200 && httpResponseCode < 300);
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
    String statusColor = lastSendSuccess ? "green" : "red";
    String statusText = lastSendSuccess ? "Connected" : "Disconnected";

    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Traffic Light Publisher</title>";
    html += "<style>body{font-family:Arial;text-align:center;margin:30px} ";
    html += "h1{color:#333} .status{margin:10px;padding:10px;border-radius:8px;background:#f4f4f4} ";
    html += ".button{font-size:18px;padding:14px 28px;margin:8px;border:none;border-radius:6px;color:white;cursor:pointer} ";
    html += ".red-btn{background:#d32f2f}.yellow-btn{background:#f9a825;color:black}.green-btn{background:#388e3c}.off-btn{background:#616161}";
    html += "</style></head><body>";
    html += "<h1>Traffic Light Publisher</h1>";
    html += "<div class='status'>Server: <span style='color:" + statusColor + "'>" + statusText + "</span><br>";
    html += "Last Command: <strong>" + lastCommand + "</strong></div>";

    html += "<div>";
    html += "<button class='button red-btn' onclick=\"sendCmd('red')\">RED</button>";
    html += "<button class='button yellow-btn' onclick=\"sendCmd('yellow')\">YELLOW</button>";
    html += "<button class='button green-btn' onclick=\"sendCmd('green')\">GREEN</button>";
    html += "<button class='button off-btn' onclick=\"sendCmd('off')\">OFF</button>";
    html += "</div>";

    html += "<script>function sendCmd(cmd){fetch('/send?state='+cmd).then(()=>setTimeout(()=>location.reload(),400));}</script>";
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
    delay(100);

    setup_wifi();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/send", HTTP_GET, handleSend);

    server.begin();
    Serial.println("Publisher HTTP server started");
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Access at: http://");
        Serial.println(WiFi.localIP());
    }
}

void loop()
{
    server.handleClient();
    delay(10);
}
