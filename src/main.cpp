#include <Arduino.h>
#include <PubSubClient.h>
#include <stdint.h>
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

}

void loop() {

  uint16_t raw = analogRead(trubidity_sensor_pin);

  uint16_t data = sensor_handler(raw);
  relay_handler(data);
  Serial.print("Raw: ");
  Serial.println(raw);
  Serial.print("percent: ");
  Serial.println(data);


}