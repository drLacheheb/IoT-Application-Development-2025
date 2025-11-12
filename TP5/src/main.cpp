#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi credentials
// TODO: Replace with your WiFi credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Adafruit IO credentials
// TODO: Replace with your Adafruit IO credentials
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "YOUR_AIO_USERNAME"
#define AIO_KEY "YOUR_AIO_KEY"

// Hardware setup
#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

// TODO: Create Adafruit MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,
                          AIO_USERNAME, AIO_KEY);

// TODO: Create feed objects (format: username/feeds/feedname)
Adafruit_MQTT_Publish temperature_feed = Adafruit_MQTT_Publish(&mqtt,
                                                               AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity_feed = Adafruit_MQTT_Publish(&mqtt,
                                                            AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe led_feed = Adafruit_MQTT_Subscribe(&mqtt,
                                                           AIO_USERNAME "/feeds/led-control");

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000; // 10 seconds

void setup_wifi()
{
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void MQTT_connect()
{
  // TODO: Implement MQTT connection with retry logic
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  // Hint: Use mqtt.connect() and check return value
  // Implement retry logic with 3 attempts
}

void ledCallback(char *data, uint16_t len)
{
  // TODO: Implement LED control callback
  Serial.print("LED Control: ");
  Serial.println(data);

  // Hint: Check if data is "ON"/"OFF" or "1"/"0"
  // Control LED_PIN accordingly
}

void publishSensorData()
{
  // TODO: Read DHT sensor and publish to Adafruit IO
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read DHT sensor!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("°C, Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  // TODO: Publish temperature and humidity to feeds
  // Hint: Use temperature_feed.publish(temp)
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("TP5 - Adafruit IO Integration");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  setup_wifi();

  // TODO: Setup LED subscription
  // Hint: led_feed.setCallback(ledCallback)
  // Hint: mqtt.subscribe(&led_feed)

  Serial.println("Setup complete!");
}

void loop()
{
  // TODO: Implement main loop
  // 1. Maintain MQTT connection
  // 2. Process incoming messages
  // 3. Ping server
  // 4. Publish sensor data periodically

  delay(100);
}
