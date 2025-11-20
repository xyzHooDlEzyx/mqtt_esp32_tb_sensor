#include <Arduino.h>

// put function declarations here:
void setup() {
  Serial.begin(115200);
  Serial.println("I`malive");
  pinMode(5, OUTPUT);

}

void loop() {
  digitalWrite(5, HIGH);
  Serial.println("Pin is HIGH");
  delay(10000);
  digitalWrite(5, LOW);
  Serial.println("Pin is LOW");
  delay(10000);

}