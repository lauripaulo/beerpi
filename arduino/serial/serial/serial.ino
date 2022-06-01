#include "SoftwareSerial.h"

void setup() {
  Serial.begin(115200);
  Serial3.begin(115200); // TX412 RX315
}

void loop() {
  if (Serial3.available()) {
    Serial.write(Serial3.read());
  }
  if (Serial.available()) {
    char resp = (char)Serial.read();
    Serial3.write(resp);
  }
}
