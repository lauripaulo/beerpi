#include <Wire.h>

// Arnakazim's TwiLiquidCrystal Arduino Library
// https://github.com/arnakazim/TwiLiquidCrystal-library
#include <TwiLiquidCrystal.h>

// Create an LCD object and setting the I2C address
#define LCD_ADDRESS 0x3f
TwiLiquidCrystal lcd(LCD_ADDRESS);

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial)
    ; // waiting for the serial monitor to become available
  // All set, we can proceed.
  Serial.println("\nBeer controller starting");
  lcd.clear();
  lcd.home();
}

void loop() {
  lcd.home();
  lcd.backlight();
  lcd.print("Hello, World!");
  delay(2000);
  lcd.noBacklight();
  lcd.clear();
  delay(2000);
  Serial.println("Loop ok.");
}

/*
 * Helper function to find first i2c address in the bus.
 */
byte list_i2c_devices() {
  int nDevices = 0;
  Serial.println("Scanning...");

  for (byte address = 1; address < 127; ++address) {
    // The i2c_scanner uses the return value of
    // the Wire.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      ++nDevices;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
  delay(5000);  // Wait 5 seconds for next scan
}
