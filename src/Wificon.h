#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EmbeddedSparkplugPayloads.h>
#include "variable.h"
#include "pins.h"


#ifndef WIFICON_H
#define WIFICON_H

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
uint8_t value = 0;

static bool turbidity_pump_value = false;
static FunctionalBasicTag* turbidity_pump_tag = nullptr;
static bool sparkplug_metric_handled = false;

static void on_turbidity_metric(bool pump_on) {
    digitalWrite(relay_pin, pump_on ? LOW : HIGH);
    Serial.print("Turbidity_pump -> ");
    Serial.println(pump_on ? "ON" : "OFF");
}

static void ensure_sparkplug_tags() {
    if (turbidity_pump_tag != nullptr) {
        return;
    }

    turbidity_pump_tag = createBoolTag("Turbidity_pump", &turbidity_pump_value, 1, true, true);
    if (turbidity_pump_tag == nullptr) {
        Serial.println("Failed to register Sparkplug tag Turbidity_pump");
        return;
    }

    readBasicTag(turbidity_pump_tag, millis());
    on_turbidity_metric(turbidity_pump_value);
}

static int handle_sparkplug_metric(BasicValue* valueReceived, FunctionalBasicTag* matchedTag) {
    if (valueReceived == nullptr || matchedTag == nullptr) {
        return 1;
    }

    if (strcmp(matchedTag->name, "Turbidity_pump") != 0) {
        return 0; 
    }

    if (valueReceived->isNull || valueReceived->datatype != spBoolean) {
        Serial.println("Turbidity_pump metric is null or not boolean");
        return 1;
    }

    if (!writeBasicTag(matchedTag, valueReceived)) {
        Serial.println("Failed to write Sparkplug metric to tag");
        return 1;
    }

    readBasicTag(matchedTag, valueReceived->timestamp);
    turbidity_pump_value = matchedTag->currentValue.value.boolValue;
    on_turbidity_metric(turbidity_pump_value);

    sparkplug_metric_handled = true;
    return 0;
}

static bool process_sparkplug_payload(byte* payload, unsigned int length) {
    ensure_sparkplug_tags();
    if (turbidity_pump_tag == nullptr) {
        return false;
    }

    sparkplug_metric_handled = false;
    bool decoded = processNCMD(reinterpret_cast<uint8_t*>(payload), length, handle_sparkplug_metric);
    if (!decoded) {
        Serial.println("Sparkplug decode failed");
        return false;
    }

    return sparkplug_metric_handled;
}


void callback(char* topic, byte* payload, unsigned int length) {
    char* group = nullptr;
    char* node = nullptr;
    char* device = nullptr;
    {
        char* tok = strtok(topic, "/");
        int slot = 0;
        while (tok != nullptr) {
            if (slot == 1) { group = tok; }
            else if (slot == 2) { node = tok; }
            else if (slot == 3) { device = tok; }
            else if (slot == 4) { device = tok; break; }
            tok = strtok(nullptr, "/");
            slot++;
        }
    }

    if (!group || strcmp(group, group_id) != 0 || !node || strcmp(node, node_id) != 0) {
        Serial.println("Topic does not target this node");
        return;
    }
    if (!device || strcmp(device, "esp32") != 0) {
        Serial.println("Skipping metric for different device");
        return;
    }

    if (process_sparkplug_payload(payload, length)) {
        return;
    }

    StaticJsonDocument<768> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
        Serial.print("JSON parse error: ");
        Serial.println(err.f_str());
        return;
    }

    JsonArray metrics = doc["metrics"].as<JsonArray>();
    if (metrics.isNull()) {
        Serial.println("JSON payload does not contain metrics array");
        return;
    }

    for (JsonObject metric : metrics) {
        const char* name = metric["name"];
        if (!name) {
            continue;
        }

        if (strcmp(name, "Turbidity_pump") == 0 && metric.containsKey("booleanValue")) {
            bool pumpOn = metric["booleanValue"];
            on_turbidity_metric(pumpOn);
        } else {
            Serial.print("Unhandled metric ");
            Serial.println(name);
        }
    }
}


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

    client.setCallback(callback);
    ensure_sparkplug_tags();
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");

            client.subscribe("spBv1.0/MyGroup/DDATA/#");
            Serial.println("Subscribed to Sparkplug topic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}


#endif 