#include "pins.h"
#ifndef RELAY_H
#define RELAY_H
void relay_handler(uint8_t data) {
  if (data > 40) {
    digitalWrite(relay_pin, LOW);
    Serial.println("Pin is LOW"); 
  }
  else {
    digitalWrite(relay_pin, HIGH);
    Serial.println("Pin is HIGH");
  }
  
}



#endif 