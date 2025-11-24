#include <Arduino.h>
#include <PubSubClient.h>
#include <stdint.h>
#include <ArduinoJson.h>
#include "Wificon.h"
#include "relay.h"
#include "sensor.h"
#include "WiFi.h"
#include "pins.h"

void setup() {
  Serial.begin(115200);
  Serial.println("I'm alive");
  pinMode(relay_pin, OUTPUT);
  pinMode(trubidity_sensor_pin, INPUT);
  setup_wifi();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  long now = millis();

  uint16_t raw = analogRead(trubidity_sensor_pin);

  uint8_t data = sensor_handler(raw);
  relay_handler(data);

  if (now - lastMsg > 3000) {
    lastMsg = now;

    StaticJsonDocument<200> doc;
    doc["sensor_percent"] = data;
    doc["relay_status"] = (data > 40) ? "LOW" : "HIGH";

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    client.publish("esp32/data", jsonBuffer);

    Serial.print("Raw: "); Serial.println(raw);
    Serial.print("Percent: "); Serial.println(data);
    Serial.print("Published JSON: "); Serial.println(jsonBuffer);
  }
}