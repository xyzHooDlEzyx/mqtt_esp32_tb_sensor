#include <WiFi.h>
#include <PubSubClient.h>
#include "variable.h"

#ifndef WIFICON_H
#define WIFICON_H

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
uint8_t value = 0;


void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;

        if (attempts > 20) {
            Serial.println("\nFailed to connect to WiFi!");
            Serial.print("WiFi status: ");
            Serial.println(WiFi.status());
            return;
        }
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqtt_server, 1883);
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}


#endif 