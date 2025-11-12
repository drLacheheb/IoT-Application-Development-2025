# TP1: ESP32 Garage Door Control with HTTP

This practical work guides you through creating a simple web server on an ESP32 that controls a garage door using HTTP commands. You'll learn how to handle HTTP GET and POST requests, control actuators (servo motor for the door), and display the door's current state on a web page.

## 1. Overview

In this TP, you will implement a garage door control system using an ESP32 board. The ESP32 will run a web server that accepts HTTP commands to open and close a simulated garage door (represented by a servo motor). The system will also provide a web interface showing the current state of the door.

### System Architecture

The system consists of:

1.  **ESP32 (Web Server):**

    - Hosts a simple HTTP web server
    - Controls a servo motor that simulates the garage door
    - Tracks the current state of the door (OPEN, CLOSED, OPENING, CLOSING)
    - Serves a web page with control buttons and status display
    - Responds to HTTP GET requests for the web interface
    - Responds to HTTP POST requests for door control commands

2.  **Hardware Components:**
    - ESP32 board
    - Servo motor (simulates garage door mechanism)
    - LED indicator (optional - shows door state)
    - Wi-Fi connection for network access

## 2. HTTP Basics for IoT Control

HTTP (Hypertext Transfer Protocol) is a request-response protocol widely used in web communication. For IoT applications, HTTP provides several advantages:

- **Universal compatibility:** Works with any web browser or HTTP client
- **Simple debugging:** Easy to test with browsers or tools like curl/Postman
- **Stateless nature:** Each request is independent, simplifying server design
- **Human-readable:** Easy to understand and troubleshoot

### HTTP Methods Used in This TP

- **GET:** Retrieve the web page and check door status

  - Example: `http://192.168.1.100/` returns the HTML interface
  - Example: `http://192.168.1.100/status` returns current door state

- **POST:** Send commands to control the door
  - Example: `http://192.168.1.100/open` triggers door opening
  - Example: `http://192.168.1.100/close` triggers door closing

## 3. Implementation Tasks

Your implementation will be in `TP1/src/main.cpp`. Follow these steps to complete the garage door control system.

### Task 1: WiFi Setup

Set up the ESP32 to connect to your Wi-Fi network.

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "YOUR_WIFI_SSID";      // TODO: Replace with your WiFi SSID
const char* password = "YOUR_PASSWORD";    // TODO: Replace with your WiFi password

WebServer server(80);  // Create server on port 80
Servo doorServo;       // Servo object for door control

// Door states
enum DoorState {
  CLOSED,
  OPENING,
  OPEN,
  CLOSING
};

DoorState currentState = CLOSED;
const int SERVO_PIN = 13;
const int LED_PIN = 2;

void setup_wifi() {
  Serial.println("Connecting to WiFi...");
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

- We use the `WiFi.h` library to connect the ESP32 to your wireless network
- The `WebServer` class handles incoming HTTP requests
- Store your network credentials in `ssid` and `password` variables

### Task 2: Servo Control Functions

Implement functions to control the servo motor position, simulating door movement.

**TODO:** Complete the servo control functions

```cpp
void openDoor() {
  if (currentState == CLOSED || currentState == CLOSING) {
    Serial.println("Opening door...");
    currentState = OPENING;
    digitalWrite(LED_PIN, HIGH);

    // TODO: Move servo from 0 to 90 degrees (closed to open)
    for (int pos = 0; pos <= 90; pos++) {
      doorServo.write(pos);
      delay(15);  // Smooth movement
    }

    currentState = OPEN;
    Serial.println("Door is now OPEN");
  }
}

void closeDoor() {
  if (currentState == OPEN || currentState == OPENING) {
    Serial.println("Closing door...");
    currentState = CLOSING;
    digitalWrite(LED_PIN, LOW);

    // TODO: Move servo from 90 to 0 degrees (open to closed)
    for (int pos = 90; pos >= 0; pos--) {
      doorServo.write(pos);
      delay(15);  // Smooth movement
    }

    currentState = CLOSED;
    Serial.println("Door is now CLOSED");
  }
}

String getStateString() {
  switch(currentState) {
    case CLOSED: return "CLOSED";
    case OPENING: return "OPENING";
    case OPEN: return "OPEN";
    case CLOSING: return "CLOSING";
    default: return "UNKNOWN";
  }
}
```

**Explanation:**

- The servo motor angle represents door position: 0° = closed, 90° = open
- We use gradual movement (loop with delays) for smooth operation
- State tracking prevents conflicting commands (can't close an already closed door)

### Task 3: HTTP Request Handlers

Implement handlers for different HTTP endpoints.

**TODO:** Implement the HTTP request handlers

#### a. Main Page Handler (GET /)

```cpp
void handleRoot() {
  // TODO: Create HTML page with door controls
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
```

**Explanation:**

- We return an HTML page with embedded CSS for styling
- JavaScript `fetch()` sends POST requests when buttons are clicked
- The page auto-reloads after commands to show updated state

#### b. Open Command Handler (POST /open)

```cpp
void handleOpen() {
  // TODO: Handle door open command
  Serial.println("Received OPEN command");
  openDoor();
  server.send(200, "text/plain", "Door opening...");
}
```

#### c. Close Command Handler (POST /close)

```cpp
void handleClose() {
  // TODO: Handle door close command
  Serial.println("Received CLOSE command");
  closeDoor();
  server.send(200, "text/plain", "Door closing...");
}
```

#### d. Status Endpoint (GET /status)

```cpp
void handleStatus() {
  // TODO: Return current door state as JSON
  String json = "{\"state\":\"" + getStateString() + "\"}";
  server.send(200, "application/json", json);
}
```

**Explanation:**

- Each handler responds to a specific URL endpoint
- We use appropriate HTTP status codes (200 = success)
- Content-Type headers indicate response format (HTML, JSON, plain text)

### Task 4: Setup and Main Loop

**TODO:** Complete the setup and loop functions

```cpp
void setup() {
  Serial.begin(115200);

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize servo
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);  // Start with door closed

  // Connect to WiFi
  setup_wifi();

  // Configure HTTP server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/open", HTTP_POST, handleOpen);
  server.on("/close", HTTP_POST, handleClose);
  server.on("/status", HTTP_GET, handleStatus);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Access the garage door control at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // TODO: Handle incoming HTTP requests
  server.handleClient();
}
```

**Explanation:**

- `server.on()` registers URL patterns and their handler functions
- `server.handleClient()` processes incoming HTTP requests
- The loop must call `handleClient()` frequently to remain responsive

## 4. Testing Your Implementation

### a. Upload and Monitor

1. Configure your WiFi credentials in the code
2. Build and upload the firmware to your ESP32
3. Open the Serial Monitor (115200 baud) to see the IP address
4. Note the IP address displayed (e.g., `192.168.1.100`)

### b. Web Interface Testing

1. Open a web browser on any device connected to the same network
2. Navigate to `http://[ESP32_IP_ADDRESS]`
3. Click "Open Door" and observe the servo movement
4. Click "Close Door" and observe the servo return to start position
5. Refresh the page to see the updated door state

### c. Command Line Testing (Optional)

Test individual endpoints using curl or PowerShell:

```powershell
# Get the main page
Invoke-WebRequest -Uri http://192.168.1.100/

# Check status
Invoke-WebRequest -Uri http://192.168.1.100/status

# Open door
Invoke-WebRequest -Uri http://192.168.1.100/open -Method POST

# Close door
Invoke-WebRequest -Uri http://192.168.1.100/close -Method POST
```

## 5. Extension Ideas (Optional)

Consider implementing these enhancements:

1. **Security:** Add basic authentication (username/password)
2. **Safety Timer:** Auto-close door after 5 minutes if left open
3. **Sensors:** Add a limit switch to detect actual door position
4. **Notifications:** Send alerts when door state changes
5. **Schedule:** Add time-based automatic open/close functionality
6. **Multi-user:** Track which user opened/closed the door with API keys

## 6. Hardware Setup (Wokwi Simulation)

The simulation includes:

- ESP32 board
- Servo motor connected to GPIO 13
- LED indicator on GPIO 2

Physical connections:

- Servo signal wire → GPIO 13
- Servo VCC → 5V (external power recommended for real hardware)
- Servo GND → GND
- LED anode → GPIO 2 (with 220Ω resistor)
- LED cathode → GND

## ✅ Deliverables

Submit the following:

1. **Code:** Completed `TP1/src/main.cpp` with all TODO sections filled
2. **Documentation:** Brief report (PDF or markdown) including:
   - Screenshots of the web interface
   - Serial monitor output showing door state changes
   - Discussion of HTTP advantages/disadvantages for IoT control
   - Response time measurements (from button click to door movement)
3. **Testing Results:** Evidence of successful door control operations

## 🧩 Grading Criteria

- **Functionality (50%):** All endpoints work correctly, servo responds to commands
- **Web Interface (20%):** Clean, functional HTML page with status display
- **State Management (15%):** Proper tracking and prevention of invalid state transitions
- **Code Quality (10%):** Clear structure, comments, error handling
- **Documentation (5%):** Complete report with observations
