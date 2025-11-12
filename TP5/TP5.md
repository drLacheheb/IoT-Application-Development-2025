# TP5: Adafruit IO Cloud Control – IoT Data Platform Integration

This practical work demonstrates how to connect an ESP32 to a cloud IoT platform (Adafruit IO) to send sensor data and receive control commands. You'll learn how to use a managed IoT service that provides data storage, visualization dashboards, and MQTT connectivity without managing your own server infrastructure.

## 1. Overview

In this TP, you will integrate an ESP32 with Adafruit IO, a free cloud platform for IoT projects. You'll send sensor readings to the cloud, visualize them on a dashboard, and control an LED remotely through the platform.

### System Architecture

The system consists of:

1.  **ESP32 Device:**

    - Reads sensor data (temperature, humidity from DHT22)
    - Publishes data to Adafruit IO via MQTT
    - Subscribes to control commands for LED
    - Maintains secure connection using TLS

2.  **Adafruit IO Cloud Platform:**

    - Receives and stores sensor data (feeds)
    - Provides web dashboard for visualization
    - Supports triggers and actions (if/then rules)
    - Manages MQTT broker with authentication
    - Free tier: 30 data points/minute, 30 days retention

3.  **Web Dashboard:**
    - Real-time charts for temperature and humidity
    - Toggle switch to control LED
    - Historical data viewing
    - Mobile-friendly interface

### Key Concepts

- **IoT Platforms:** Managed services for device connectivity and data management
- **Feeds:** Data streams in Adafruit IO (like MQTT topics)
- **MQTT over TLS:** Secure communication with cloud broker
- **Dashboards:** Visual interfaces for monitoring and control
- **API Keys:** Authentication for device access

## 2. Why Use Cloud IoT Platforms?

### Advantages

- **No Server Management:** Focus on devices, not infrastructure
- **Built-in Features:** Dashboards, data storage, APIs included
- **Scalability:** Handles many devices automatically
- **Reliability:** Professional hosting with uptime guarantees
- **Security:** TLS encryption and authentication built-in
- **Mobile Access:** Control devices from anywhere

### Disadvantages

- **Cost:** Free tiers are limited; scaling requires payment
- **Vendor Lock-in:** Platform-specific APIs and data formats
- **Internet Dependency:** Requires stable connectivity
- **Privacy:** Data stored on third-party servers
- **Rate Limits:** Throttling on free plans

### Adafruit IO vs Alternatives

| Platform             | Free Tier               | Best For                  |
| -------------------- | ----------------------- | ------------------------- |
| **Adafruit IO**      | 30 pts/min, easy to use | Learning, hobby projects  |
| **ThingSpeak**       | 3M msgs/year            | Data logging and analysis |
| **Blynk**            | 1 device                | Mobile app control        |
| **AWS IoT Core**     | 12 months free trial    | Production systems        |
| **Google Cloud IoT** | Pay-as-you-go           | Enterprise integration    |

## 3. Getting Started with Adafruit IO

### Step 1: Create Adafruit IO Account

1. Visit https://io.adafruit.com/
2. Click "Get Started for Free"
3. Create an account (free tier is sufficient)
4. Verify your email address

### Step 2: Get Your Credentials

1. After logging in, click the **Key** icon (🔑) in the top menu
2. Note your:
   - **Username:** (e.g., `your_username`)
   - **Active Key:** (long hexadecimal string)
3. Keep these secure – they're like passwords!

### Step 3: Create Feeds

Feeds are data channels for your sensors and controls.

1. Click **Feeds** in the left menu
2. Click **+ New Feed**
3. Create three feeds:
   - `temperature` (for temperature readings)
   - `humidity` (for humidity readings)
   - `led-control` (for LED on/off commands)
4. Note the feed keys (usually lowercase versions of names)

### Step 4: Create Dashboard

1. Click **Dashboards** in the left menu
2. Click **+ New Dashboard**
3. Name it "TP5 - ESP32 Monitor"
4. Click on the dashboard to open it
5. Add blocks:
   - **Line Chart** for temperature feed
   - **Line Chart** for humidity feed
   - **Toggle** for led-control feed

## 4. Implementation Guide

## Part A: ESP32 Firmware

### Task 1: Library Installation and Setup

**File:** `TP5/platformio.ini`

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit MQTT Library @ ^2.5.0
    adafruit/DHT sensor library
    adafruit/Adafruit Unified Sensor
```

### Task 2: Configuration and Includes

**File:** `TP5/src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi credentials
// TODO: Replace with your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Adafruit IO credentials
// TODO: Replace with your Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR_AIO_USERNAME"
#define AIO_KEY         "YOUR_AIO_KEY"

// Hardware setup
#define DHTPIN 4          // DHT sensor on GPIO 4
#define DHTTYPE DHT22     // DHT22 sensor
#define LED_PIN 2         // Built-in LED or external LED

DHT dht(DHTPIN, DHTTYPE);

// WiFi client
WiFiClient client;

// Adafruit MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,
                          AIO_USERNAME, AIO_KEY);

// Feeds (topics)
// TODO: Create feed objects for publishing and subscribing
// Format: username/feeds/feedname
Adafruit_MQTT_Publish temperature_feed = Adafruit_MQTT_Publish(&mqtt,
                                          AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity_feed = Adafruit_MQTT_Publish(&mqtt,
                                       AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe led_feed = Adafruit_MQTT_Subscribe(&mqtt,
                                    AIO_USERNAME "/feeds/led-control");

// Timing
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;  // 10 seconds
```

**Explanation:**

- We use the Adafruit MQTT library which handles connection management
- Feed paths follow the format: `username/feeds/feedname`
- Publishing interval is 10 seconds to stay within rate limits
- TLS can be enabled by using `WiFiClientSecure` instead of `WiFiClient`

### Task 3: WiFi Connection

**TODO:** Implement WiFi connection function

```cpp
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
```

### Task 4: MQTT Connection Management

**TODO:** Implement MQTT connection function with reconnection logic

```cpp
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {  // connect returns 0 on success
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      Serial.println("MQTT connection failed! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("MQTT Connected!");
}
```

**Explanation:**

- Adafruit MQTT library provides `connect()` method that returns 0 on success
- We implement retry logic with exponential backoff
- After 3 failed attempts, we restart the ESP32
- `connectErrorString()` helps debug connection issues

### Task 5: LED Control Callback

**TODO:** Implement callback function for LED control

```cpp
void ledCallback(char *data, uint16_t len) {
  Serial.print("LED Control: ");
  Serial.println(data);

  // Convert to string for easier comparison
  String command = String(data);
  command.toLowerCase();

  if (command == "on" || command == "1") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED turned ON");
  } else if (command == "off" || command == "0") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED turned OFF");
  } else {
    Serial.println("Unknown command");
  }
}
```

**Explanation:**

- Callback is triggered when a message arrives on the subscribed feed
- We handle both text ("on"/"off") and numeric ("1"/"0") commands
- This flexibility makes the dashboard toggle work properly

### Task 6: Sensor Reading and Publishing

**TODO:** Implement function to read sensors and publish to Adafruit IO

```cpp
void publishSensorData() {
  // Read sensor data
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Check if readings are valid
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C, Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  // Publish to Adafruit IO
  if (!temperature_feed.publish(temp)) {
    Serial.println("Failed to publish temperature");
  } else {
    Serial.println("Temperature published!");
  }

  // Small delay between publishes
  delay(1000);

  if (!humidity_feed.publish(hum)) {
    Serial.println("Failed to publish humidity");
  } else {
    Serial.println("Humidity published!");
  }
}
```

**Explanation:**

- `isnan()` checks for invalid sensor readings
- We add a 1-second delay between publishes to avoid rate limiting
- `publish()` returns `true` on success
- Separate publishes for each feed allow independent visualization

### Task 7: Setup Function

**TODO:** Complete the setup function

```cpp
void setup() {
  Serial.begin(115200);
  delay(2000);  // Give time for serial monitor to open

  Serial.println("TP5 - Adafruit IO Integration");
  Serial.println("==============================");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Connect to WiFi
  setup_wifi();

  // Setup LED subscription callback
  led_feed.setCallback(ledCallback);

  // Subscribe to LED control feed
  mqtt.subscribe(&led_feed);

  Serial.println("Setup complete!");
}
```

**Explanation:**

- We set up the LED callback before subscribing
- `mqtt.subscribe()` registers our interest in the feed
- The callback will be called automatically when messages arrive

### Task 8: Main Loop

**TODO:** Implement the main loop with periodic publishing and subscription processing

```cpp
void loop() {
  // Maintain MQTT connection
  MQTT_connect();

  // Process incoming messages (non-blocking)
  // This calls callbacks for subscribed feeds
  mqtt.processPackets(10);  // Process for up to 10ms

  // Ping the server to keep connection alive
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

  // Publish sensor data periodically
  unsigned long currentTime = millis();
  if (currentTime - lastPublish >= PUBLISH_INTERVAL) {
    publishSensorData();
    lastPublish = currentTime;
  }

  // Small delay
  delay(100);
}
```

**Explanation:**

- `processPackets()` handles incoming messages (including LED commands)
- `ping()` keeps the connection alive (MQTT requirement)
- We use `millis()` for non-blocking timing
- The loop runs continuously, checking for messages and publishing periodically

## 5. Dashboard Configuration

### Temperature Chart Setup

1. On your dashboard, click the temperature chart block
2. Settings:
   - **Feed:** temperature
   - **Block Type:** Line Chart
   - **Y-Axis Min:** 0
   - **Y-Axis Max:** 50
   - **Time Range:** 1 hour

### Humidity Chart Setup

1. Click the humidity chart block
2. Settings:
   - **Feed:** humidity
   - **Block Type:** Line Chart
   - **Y-Axis Min:** 0
   - **Y-Axis Max:** 100
   - **Time Range:** 1 hour

### LED Toggle Setup

1. Click the LED control toggle block
2. Settings:
   - **Feed:** led-control
   - **Block Type:** Toggle
   - **On Text:** "ON"
   - **Off Text:** "OFF"
   - **Button On Value:** ON
   - **Button Off Value:** OFF

## 6. Testing Your Implementation

### Step 1: Upload Firmware

1. Update credentials in `main.cpp`
2. Build and upload to ESP32
3. Open Serial Monitor (115200 baud)

### Step 2: Verify Connection

Monitor serial output for:

```
WiFi connected
IP address: 192.168.1.xxx
Connecting to MQTT...
MQTT Connected!
Temperature: 23.5°C, Humidity: 55.2%
Temperature published!
Humidity published!
```

### Step 3: View Dashboard

1. Open your Adafruit IO dashboard
2. Observe temperature and humidity charts updating
3. Wait for multiple data points to see the trend lines

### Step 4: Test LED Control

1. Click the toggle switch on the dashboard
2. Observe:
   - Serial monitor shows "LED Control: ON"
   - Physical LED turns on
3. Toggle off and verify LED turns off

### Step 5: Test Rate Limits

- Free tier allows 30 data points per minute (2 seconds minimum between publishes)
- Current config: 10 seconds between publishes = well within limits
- If you exceed limits, data will be queued or dropped

## 7. Extension Ideas

1. **Additional Sensors:** Add more feeds for light sensor, motion sensor, etc.
2. **Triggers:** Use Adafruit IO triggers (if temperature > 30°C, send notification)
3. **Data Export:** Use Adafruit IO API to download historical data for analysis
4. **TLS Encryption:** Switch to secure MQTT (port 8883)
5. **Multiple Devices:** Connect multiple ESP32s to the same dashboard
6. **Local Fallback:** Store data locally if internet connection is lost
7. **Battery Monitoring:** Add a feed for ESP32 battery voltage
8. **Geolocation:** Use Adafruit IO's map block to show device location

## 8. Troubleshooting

### Common Issues

1. **"MQTT Connection Failed"**

   - Verify username and key are correct (no spaces)
   - Check internet connectivity
   - Ensure feeds exist in Adafruit IO
   - Try port 1883 (non-TLS) first

2. **"Failed to publish"**

   - Check rate limits (30 data points/minute max)
   - Verify feed names match exactly (case-sensitive)
   - Ensure MQTT connection is active

3. **Dashboard not updating**

   - Refresh the browser page
   - Check feed history to see if data is arriving
   - Verify feed is added to the dashboard block

4. **LED not responding to toggle**

   - Check callback is registered: `led_feed.setCallback(ledCallback)`
   - Verify subscription: `mqtt.subscribe(&led_feed)`
   - Ensure `processPackets()` is called in loop

5. **WiFi connection drops**
   - Add WiFi reconnection logic in loop
   - Check router signal strength
   - Use WiFi watchdog timer

## ✅ Deliverables

Submit the following:

1. **Code Files:**

   - `TP5/src/main.cpp` (complete implementation)
   - `TP5/platformio.ini`

2. **Documentation (PDF or Markdown):**

   - Screenshots of Adafruit IO dashboard with live data
   - Serial monitor output showing successful connections and publishes
   - Description of feed configuration and dashboard blocks
   - Discussion of cloud IoT platforms: advantages and limitations
   - Comparison: Self-hosted MQTT (TP4) vs Adafruit IO

3. **Adafruit IO Configuration:**
   - List of feeds created
   - Dashboard JSON export (optional)

## 🧩 Grading Criteria

- **MQTT Connection (25%):** Successful connection to Adafruit IO
- **Data Publishing (25%):** Temperature and humidity data appearing on dashboard
- **Remote Control (25%):** LED responding to dashboard toggle
- **Code Quality (15%):** Error handling, comments, structure
- **Documentation (10%):** Complete report with screenshots and analysis
