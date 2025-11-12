# TP3: ESP32-Server-ESP32 Indirect Communication – Traffic Light Control

This practical work demonstrates indirect device-to-device communication where two ESP32 boards communicate through a central web server. One ESP32 publishes traffic light commands to the server, while another ESP32 polls the server to read and execute these commands. This pattern is fundamental for building scalable IoT systems where direct connectivity isn't feasible.

## 1. Overview

In this TP, you will implement a three-component system where ESP32 devices communicate indirectly via a central server. This architecture is more scalable than direct communication and mirrors real-world IoT deployments.

### System Architecture

The system consists of three components:

1.  **ESP32 Publisher (Command Sender):**

    - Provides a web interface with traffic light control buttons
    - Sends HTTP POST requests to the server with commands
    - Does NOT communicate directly with the traffic light ESP32

2.  **Central HTTP Server (Python/Flask):**

    - Stores the current traffic light command in memory
    - Provides POST endpoint `/update` for the publisher to send commands
    - Provides GET endpoint `/command` for the traffic light to poll for commands
    - Acts as a message broker using simple HTTP

3.  **ESP32 Subscriber (Traffic Light):**
    - Polls the server periodically to check for new commands
    - Controls three LEDs based on received commands
    - Updates its state without knowing who sent the command

### Key Concepts

- **Decoupling:** Publisher and subscriber don't need to know each other's addresses
- **Server-Mediated Communication:** Central server manages state and message routing
- **Polling vs Push:** Subscriber actively requests data (polling) rather than receiving it (push)
- **Scalability:** Multiple publishers or subscribers can connect to the same server

## 2. Why Indirect Communication?

### Advantages over Direct Communication (TP2)

- **Scalability:** Supports multiple publishers and subscribers without complex routing
- **Decoupling:** Devices don't need to know each other's IP addresses
- **Persistence:** Server can store messages even if subscriber is temporarily offline
- **Flexibility:** Easy to add logging, authentication, or message transformation at the server
- **NAT Traversal:** Works across different networks (if server is public)

### Disadvantages

- **Latency:** Additional round trip through server increases response time
- **Server Dependency:** System fails if server goes down (single point of failure)
- **Complexity:** Three components to develop and maintain
- **Bandwidth:** More network traffic due to polling

### Comparison to MQTT (TP4)

This pattern is similar to MQTT but simpler:

- MQTT uses persistent connections and push notifications
- This TP uses polling with HTTP GET requests
- MQTT is more efficient for high-frequency updates
- HTTP polling is simpler to implement and debug

## 3. Implementation Guide

## Part A: Central HTTP Server (Python + Flask)

The server stores the current traffic light state and provides endpoints for publishing and subscribing.

### Task 1: Server Setup

**File:** `TP3/server/traffic_server.py`

```python
from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

# In-memory storage for traffic light state
current_command = {
    "state": "off",
    "timestamp": datetime.now().isoformat(),
    "source": "system"
}

@app.route('/')
def home():
    """
    Simple web page showing server status
    """
    html = f"""
    <html>
    <head><title>Traffic Light Server</title></head>
    <body>
        <h1>Traffic Light Control Server</h1>
        <h2>Current State: {current_command['state'].upper()}</h2>
        <p>Last updated: {current_command['timestamp']}</p>
        <p>Source: {current_command['source']}</p>
        <hr>
        <h3>API Endpoints:</h3>
        <ul>
            <li>POST /update - Update traffic light command</li>
            <li>GET /command - Get current command</li>
        </ul>
    </body>
    </html>
    """
    return html

@app.route('/update', methods=['POST'])
def update_command():
    """
    TODO: Implement endpoint to receive new commands from publisher ESP32
    Expected JSON: {"state": "red|yellow|green|off", "source": "device_id"}
    """
    global current_command

    try:
        data = request.get_json()

        if not data or 'state' not in data:
            return jsonify({"error": "Missing 'state' field"}), 400

        # Validate state value
        valid_states = ['red', 'yellow', 'green', 'off']
        if data['state'].lower() not in valid_states:
            return jsonify({"error": f"Invalid state. Must be one of: {valid_states}"}), 400

        # Update current command
        current_command = {
            "state": data['state'].lower(),
            "timestamp": datetime.now().isoformat(),
            "source": data.get('source', 'unknown')
        }

        print(f"[UPDATE] New command: {current_command['state']} from {current_command['source']}")

        return jsonify({
            "success": True,
            "command": current_command
        }), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/command', methods=['GET'])
def get_command():
    """
    TODO: Implement endpoint for subscriber ESP32 to poll for commands
    Returns current traffic light state as JSON
    """
    return jsonify(current_command), 200

if __name__ == '__main__':
    print("Starting Traffic Light Control Server...")
    print("Publisher POST to: http://localhost:5000/update")
    print("Subscriber GET from: http://localhost:5000/command")
    app.run(host='0.0.0.0', port=5000, debug=True)
```

**Explanation:**

- `current_command` is a simple in-memory store (not persistent across restarts)
- `/update` validates incoming commands and updates the state
- `/command` returns the current state for polling
- For production, you'd use a database for persistence

### Task 2: Server Dependencies

**File:** `TP3/server/requirements.txt`

```
Flask==3.0.0
```

### Task 3: Running the Server

```powershell
# Install dependencies
cd TP3/server
pip install -r requirements.txt

# Run the server
python traffic_server.py
```

The server will start on `http://localhost:5000` (or your machine's IP for network access).

## Part B: ESP32 Publisher (Command Sender)

The publisher sends traffic light commands to the server.

### Task 1: Basic Setup

**File:** `TP3/publisher/src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// TODO: Configure your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// TODO: Set your server address (replace with your computer's IP)
const char* serverURL = "http://192.168.1.10:5000";

WebServer server(80);
String lastCommand = "none";
bool lastSendSuccess = false;

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
```

### Task 2: Send Command Function

**TODO:** Implement function to send commands to the server

```cpp
bool sendCommandToServer(String state) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return false;
  }

  HTTPClient http;
  String url = String(serverURL) + "/update";

  // TODO: Create JSON payload with state and device ID
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

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print("Error sending command. Code: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}
```

**Explanation:**

- We use `ArduinoJson` to create properly formatted JSON
- The payload includes the desired state and a source identifier
- HTTP response codes help diagnose issues (200 = success, 4xx/5xx = errors)

### Task 3: Web Interface

**TODO:** Create a web interface for sending commands

```cpp
void handleRoot() {
  String statusColor = lastSendSuccess ? "green" : "red";
  String statusText = lastSendSuccess ? "Connected" : "Disconnected";

  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Traffic Light Publisher</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html += "h1 { color: #333; }";
  html += ".status { margin: 20px; padding: 15px; background-color: #f0f0f0; border-radius: 10px; }";
  html += ".button { font-size: 20px; padding: 20px 40px; margin: 10px; ";
  html += "border: none; border-radius: 5px; color: white; cursor: pointer; }";
  html += ".red-btn { background-color: #ff0000; }";
  html += ".yellow-btn { background-color: #ffcc00; color: black; }";
  html += ".green-btn { background-color: #00ff00; color: black; }";
  html += ".off-btn { background-color: #666666; }";
  html += "</style></head><body>";
  html += "<h1>Traffic Light Publisher</h1>";
  html += "<div class='status'>";
  html += "Server: <span style='color: " + statusColor + "'>" + statusText + "</span><br>";
  html += "Last Command: <strong>" + lastCommand + "</strong>";
  html += "</div>";
  html += "<button class='button red-btn' onclick='sendCmd(\"red\")'>RED</button><br>";
  html += "<button class='button yellow-btn' onclick='sendCmd(\"yellow\")'>YELLOW</button><br>";
  html += "<button class='button green-btn' onclick='sendCmd(\"green\")'>GREEN</button><br>";
  html += "<button class='button off-btn' onclick='sendCmd(\"off\")'>OFF</button>";
  html += "<script>";
  html += "function sendCmd(cmd) {";
  html += "  fetch('/send?state=' + cmd)";
  html += "    .then(response => response.text())";
  html += "    .then(() => setTimeout(() => location.reload(), 500));";
  html += "}";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleSend() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    lastSendSuccess = sendCommandToServer(state);
    lastCommand = state;

    if (lastSendSuccess) {
      server.send(200, "text/plain", "Command sent to server");
    } else {
      server.send(500, "text/plain", "Failed to send command");
    }
  } else {
    server.send(400, "text/plain", "Missing state parameter");
  }
}
```

### Task 4: Main Setup and Loop

**TODO:** Complete setup and loop

```cpp
void setup() {
  Serial.begin(115200);

  setup_wifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_GET, handleSend);

  server.begin();
  Serial.println("Publisher HTTP server started");
  Serial.print("Access at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
```

## Part C: ESP32 Subscriber (Traffic Light)

The subscriber polls the server and controls LEDs based on commands.

### Task 1: Basic Setup

**File:** `TP3/subscriber/src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// TODO: Configure your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// TODO: Set your server address
const char* serverURL = "http://192.168.1.10:5000";

// LED pins
const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

// Polling interval (milliseconds)
const unsigned long POLL_INTERVAL = 2000;  // 2 seconds
unsigned long lastPollTime = 0;

String currentState = "off";

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
```

### Task 2: LED Control Functions

```cpp
void setRedLight() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  Serial.println("RED light ON");
}

void setYellowLight() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  Serial.println("YELLOW light ON");
}

void setGreenLight() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  Serial.println("GREEN light ON");
}

void allLightsOff() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  Serial.println("All lights OFF");
}

void applyState(String state) {
  if (state == "red") {
    setRedLight();
  } else if (state == "yellow") {
    setYellowLight();
  } else if (state == "green") {
    setGreenLight();
  } else if (state == "off") {
    allLightsOff();
  }
  currentState = state;
}
```

### Task 3: Polling Function

**TODO:** Implement function to poll server for commands

```cpp
void pollServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  String url = String(serverURL) + "/command";

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();

    // Parse JSON response
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      String newState = doc["state"].as<String>();
      String timestamp = doc["timestamp"].as<String>();

      // Only update if state has changed
      if (newState != currentState) {
        Serial.print("New command received: ");
        Serial.println(newState);
        applyState(newState);
      }
    } else {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Error polling server. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
```

**Explanation:**

- We parse the JSON response from the server
- Only update LEDs if the state has actually changed
- This reduces unnecessary LED operations

### Task 4: Main Setup and Loop

**TODO:** Complete setup and loop with polling logic

```cpp
void setup() {
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

void loop() {
  unsigned long currentTime = millis();

  // Poll server at regular intervals
  if (currentTime - lastPollTime >= POLL_INTERVAL) {
    pollServer();
    lastPollTime = currentTime;
  }

  // Small delay to prevent CPU overload
  delay(10);
}
```

**Explanation:**

- We use `millis()` for non-blocking timing
- Polling every 2 seconds balances responsiveness and network load
- Adjust `POLL_INTERVAL` based on your latency requirements

## 4. Testing Your Implementation

### Step 1: Start the Server

```powershell
cd TP3/server
python traffic_server.py
```

Note your computer's IP address (e.g., `192.168.1.10`).

### Step 2: Update ESP32 Code

Update both `publisher.cpp` and `subscriber.cpp` with:

- Your WiFi credentials
- Your server's IP address in `serverURL`

### Step 3: Upload Firmware

1. Upload `subscriber.cpp` to the first ESP32 (traffic light)
2. Upload `publisher.cpp` to the second ESP32 (controller)

### Step 4: Test Communication

1. Open the publisher's web interface in a browser
2. Click "RED" button
3. Observe:
   - Server console shows the update
   - Subscriber ESP32 polls and changes LED to red
   - Publisher shows "Connected" status

### Step 5: Monitor Latency

- Measure time from button click to LED change
- Typical latency: 0-2 seconds (depending on poll interval)

## 5. Extension Ideas

1. **WebSockets:** Replace polling with WebSocket push for real-time updates
2. **Command History:** Store last 10 commands in server with timestamps
3. **Multiple Subscribers:** Add multiple traffic light ESP32s
4. **Authentication:** Add API keys for publishers
5. **Database:** Use SQLite or PostgreSQL for persistent storage
6. **Dashboard:** Add real-time monitoring dashboard to server
7. **Message Queue:** Implement a queue for commands (FIFO)

## ✅ Deliverables

Submit the following:

1. **Code Files:**

   - `TP3/server/traffic_server.py`

- `TP3/publisher/src/main.cpp`
- `TP3/subscriber/src/main.cpp`

2. **Documentation (PDF or Markdown):**

   - System architecture diagram
   - Screenshots of: server web page, publisher interface, subscriber serial output
   - Latency measurements and analysis
   - Comparison table: Direct (TP2) vs Indirect (TP3) communication

3. **Testing Results:**
   - Evidence of successful command routing
   - Server logs showing updates and polls

## 🧩 Grading Criteria

- **Server Implementation (25%):** Correct endpoints, state management, validation
- **Publisher (25%):** Successful command sending, proper JSON formatting
- **Subscriber (25%):** Correct polling, JSON parsing, LED control
- **System Integration (15%):** All three components work together
- **Documentation (10%):** Complete report with analysis
