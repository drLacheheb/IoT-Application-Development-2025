#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const char *ssid = "Wokwi-GUEST";
const char *password = "";

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "YOUR_AIO_USERNAME"
#define AIO_KEY "YOUR_AIO_KEY"

#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,
                          AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temperature_feed = Adafruit_MQTT_Publish(&mqtt,
                                                               AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity_feed = Adafruit_MQTT_Publish(&mqtt,
                                                            AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe led_feed = Adafruit_MQTT_Subscribe(&mqtt,
                                                           AIO_USERNAME "/feeds/led-control");

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;

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
  int8_t ret;

  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0)
    {
      Serial.println("MQTT failed! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("MQTT Connected!");
}

void ledCallback(char *data, uint16_t len)
{
  Serial.print("LED Control: ");
  Serial.println(data);

  String command = String(data);
  command.toLowerCase();

  if (command == "on" || command == "1")
  {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
  }
  else if (command == "off" || command == "0")
  {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
  }
}

void publishSensorData()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read DHT!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("°C, Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  if (!temperature_feed.publish(temp))
  {
    Serial.println("Failed to publish temp");
  }
  else
  {
    Serial.println("Temp published!");
  }

  delay(1000);

  if (!humidity_feed.publish(hum))
  {
    Serial.println("Failed to publish humidity");
  }
  else
  {
    Serial.println("Humidity published!");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("TP5 - Adafruit IO");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  setup_wifi();

  led_feed.setCallback(ledCallback);
  mqtt.subscribe(&led_feed);

  Serial.println("Setup complete!");
}

void loop()
{
  MQTT_connect();

  mqtt.processPackets(10);

  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }

  unsigned long currentTime = millis();
  if (currentTime - lastPublish >= PUBLISH_INTERVAL)
  {
    publishSensorData();
    lastPublish = currentTime;
  }

  delay(100);
}
