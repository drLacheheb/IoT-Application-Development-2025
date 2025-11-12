# TP2: ESP32-to-ESP32 Direct Communication – Traffic Light Control

This practical work demonstrates direct device-to-device communication between two ESP32 boards using HTTP. One ESP32 acts as a controller sending commands, while the other acts as a traffic light system with three LEDs (red, yellow, green). This TP teaches you how embedded systems can communicate directly without requiring a central server.

## 1. Overview

In this TP, you will implement a peer-to-peer communication system where two ESP32 boards communicate directly over a local network. This pattern is useful for distributed IoT systems where devices need to coordinate without cloud infrastructure.

### System Architecture

The system consists of two ESP32 boards:

1.  **ESP32 Controller (Sender):**

    - Hosts a simple web interface with three buttons (Red, Yellow, Green)
    - Sends HTTP POST requests directly to the Traffic Light ESP32
    - Displays the last command sent
    - Monitors connection status

2.  **ESP32 Traffic Light (Receiver):**
    - Runs an HTTP server listening for light commands
    - Controls three LEDs (red on GPIO 25, yellow on GPIO 26, green on GPIO 27)
    - Responds to commands: `/red`, `/yellow`, `/green`, `/off`
    - Maintains current light state

### Key Concepts

- **Direct IP Communication:** Devices communicate using static IP addresses on the same network
- **RESTful API:** Simple HTTP endpoints for controlling hardware
- **Distributed Control:** No central server required
- **Real-time Response:** Low latency device-to-device commands

## 2. Why Direct ESP-to-ESP Communication?

Direct communication between ESP32 devices has several advantages:

### Advantages

- **Low Latency:** No intermediate server means faster response times
- **Offline Operation:** Works without internet connectivity
- **Simplicity:** Fewer components to manage and configure
- **Cost Effective:** No cloud hosting or broker subscription needed
- **Privacy:** Data stays on local network

### Disadvantages

- **Scalability:** Difficult to manage many devices
- **Fixed Addressing:** Requires static IPs or mDNS
- **No Persistence:** No message queuing if receiver is offline
- **Limited Range:** Restricted to local network unless using VPN/tunneling

### When to Use

- Local automation systems (home/office)
- Paired devices (remote and actuator)
- Low-latency control applications
- Development and testing environments

## 3. Implementation Guide

You will implement two separate firmware programs for the two ESP32 boards.

## Part A: Traffic Light ESP32 (Receiver)

This ESP32 receives commands and controls the LED traffic light.

### Hardware Setup

- **Red LED:** GPIO 25 (with 220Ω resistor)
- **Yellow LED:** GPIO 26 (with 220Ω resistor)
- **Green LED:** GPIO 27 (with 220Ω resistor)

### Task 1: Basic Setup and WiFi Connection

**File:** `TP2/traffic_light/src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// TODO: Configure your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// TODO: Set a static IP for this device
IPAddress local_IP(192, 168, 1, 100);    // Traffic Light IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// LED pins
const int RED_LED = 25;
const int YELLOW_LED = 26;
const int GREEN_LED = 27;

void setup_wifi() {
  Serial.println("Configuring static IP...");

  // TODO: Configure static IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP configuration failed!");
  }

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

**Explanation:**

- We use `WiFi.config()` to set a static IP address for predictable addressing
- The traffic light will always be accessible at `192.168.1.100`
- Adjust the IP range to match your network configuration

### Task 2: LED Control Functions

**TODO:** Implement functions to control the traffic light LEDs

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
```

**Explanation:**

- Each function ensures only one LED is active at a time
- This prevents invalid states (multiple lights on simultaneously)

### Task 3: HTTP Request Handlers

**TODO:** Create handlers for each light command

```cpp
void handleRed() {
  setRedLight();
  server.send(200, "text/plain", "Red light activated");
}

void handleYellow() {
  setYellowLight();
  server.send(200, "text/plain", "Yellow light activated");
}

void handleGreen() {
  setGreenLight();
  server.send(200, "text/plain", "Green light activated");
}

void handleOff() {
  allLightsOff();
  server.send(200, "text/plain", "All lights off");
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Traffic Light System</h1>";
  html += "<p>Listening for commands...</p>";
  html += "<p>Endpoints: /red, /yellow, /green, /off</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}
```

**Explanation:**

- Each endpoint triggers the corresponding LED state
- The root endpoint provides documentation for available commands

### Task 4: Setup and Main Loop

**TODO:** Complete the setup and loop functions

```cpp
void setup() {
  Serial.begin(115200);

  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  allLightsOff();

  // Connect to WiFi
  setup_wifi();

  // Configure server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/red", HTTP_POST, handleRed);
  server.on("/yellow", HTTP_POST, handleYellow);
  server.on("/green", HTTP_POST, handleGreen);
  server.on("/off", HTTP_POST, handleOff);

  // Start server
  server.begin();
  Serial.println("Traffic Light HTTP server started");
}

void loop() {
  server.handleClient();
}
```

## Part B: Controller ESP32 (Sender)

This ESP32 provides a web interface to send commands to the traffic light.

### Task 1: Setup and Configuration

**File:** `TP2/controller/src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// TODO: Configure your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// TODO: Set the traffic light ESP32's IP address
const char* trafficLightIP = "http://192.168.1.100";

WebServer server(80);
String lastCommand = "None";

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

**Explanation:**

- The controller uses DHCP (dynamic IP) since we only need to know the traffic light's address
- Store the traffic light's IP as a constant for easy updates

### Task 2: HTTP Client Functions

**TODO:** Implement functions to send commands to the traffic light

```cpp
bool sendCommand(String command) {
  if (WiFi.status() != WL_CONNECTED) {
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

- `HTTPClient` manages the HTTP POST request
- We check for WiFi connection before attempting to send
- Response codes help diagnose connection issues (200 = success, negative = error)

### Task 3: Web Interface Handlers

**TODO:** Create a web interface for controlling the traffic light

```cpp
void handleRoot() {
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

void handleSend() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    bool success = sendCommand(cmd);

    if (success) {
      lastCommand = cmd;
      server.send(200, "text/plain", "Command sent successfully");
    } else {
      server.send(500, "text/plain", "Failed to send command");
    }
  } else {
    server.send(400, "text/plain", "Missing command parameter");
  }
}
```

**Explanation:**

- The web interface has four buttons corresponding to light states
- JavaScript `fetch()` calls the `/send` endpoint with the command parameter
- The ESP32 then forwards this command to the traffic light ESP32

### Task 4: Setup and Main Loop

**TODO:** Complete the setup and loop for the controller

```cpp
void setup() {
  Serial.begin(115200);

  setup_wifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_GET, handleSend);

  server.begin();
  Serial.println("Controller HTTP server started");
  Serial.print("Access the controller at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
```

## 4. Testing Your Implementation

### Step 1: Upload Traffic Light Firmware

1. Compile and upload `TP2/traffic_light/src/main.cpp` to the first ESP32
2. Open Serial Monitor and note the IP address (should be 192.168.1.100)
3. Verify the server starts successfully

### Step 2: Upload Controller Firmware

1. Compile and upload `TP2/controller/src/main.cpp` to the second ESP32
2. Open Serial Monitor and note the controller's IP address
3. Verify WiFi connection is successful

### Step 3: Test Communication

1. Open a web browser and navigate to the controller's IP address
2. Click each button (Red, Yellow, Green, Off)
3. Observe the traffic light ESP32's LEDs changing
4. Monitor both Serial outputs for debugging information

### Step 4: Direct Testing (Optional)

Test the traffic light directly without the controller:

```powershell
# Test red light
Invoke-WebRequest -Uri http://192.168.1.100/red -Method POST

# Test yellow light
Invoke-WebRequest -Uri http://192.168.1.100/yellow -Method POST

# Test green light
Invoke-WebRequest -Uri http://192.168.1.100/green -Method POST

# Turn off all lights
Invoke-WebRequest -Uri http://192.168.1.100/off -Method POST
```

## 5. Troubleshooting

### Common Issues

1. **"Failed to send command" error:**

   - Verify both ESP32s are on the same network
   - Check the traffic light's IP address in the controller code
   - Ensure the traffic light ESP32 is powered and running

2. **Static IP not working:**

   - Verify the IP range matches your network (e.g., 192.168.1.x or 192.168.0.x)
   - Check your router's DHCP range doesn't conflict with static IPs
   - Some networks require reserving IPs in router settings

3. **LEDs not responding:**

   - Check wiring connections and resistors
   - Verify GPIO pin numbers in code match physical connections
   - Test LEDs individually with `digitalWrite()` commands

4. **Timeout errors:**
   - Increase timeout in `HTTPClient.setTimeout(5000)` (5 seconds)
   - Check network signal strength
   - Verify firewall settings aren't blocking ESP32 traffic

## 6. Extension Ideas

1. **Automatic Sequence:** Implement a traffic light cycle (Red→Yellow→Green) with timers
2. **Status Feedback:** Have the traffic light send its current state back to the controller
3. **Multi-Controller:** Allow multiple controllers to manage the same traffic light
4. **Emergency Mode:** Add a button for emergency all-red state
5. **mDNS:** Use mDNS (`trafficlight.local`) instead of static IPs for easier setup
6. **Bi-directional:** Add sensors to the traffic light that report back to the controller

## ✅ Deliverables

Submit the following:

1. **Code Files:**

   - `TP2/traffic_light/src/main.cpp` (complete receiver code)
   - `TP2/controller/src/main.cpp` (complete sender code)

2. **Documentation (PDF or Markdown):**

   - Network diagram showing IP addresses
   - Screenshots of the web interface
   - Serial monitor outputs from both ESP32s
   - Response time measurements (button click to LED change)
   - Discussion of direct communication advantages/disadvantages

3. **Demonstration Video (Optional):**
   - Show button clicks controlling the traffic light
   - Display both serial monitors simultaneously

## 🧩 Grading Criteria

- **Functionality (50%):** All commands work, LEDs respond correctly
- **Network Configuration (20%):** Static IP setup, reliable communication
- **User Interface (15%):** Clean, functional controller web page
- **Code Quality (10%):** Clear structure, comments, error handling
- **Documentation (5%):** Complete report with measurements and analysis
