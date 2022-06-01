#include <Wire.h>

// Arnakazim's TwiLiquidCrystal Arduino Library
// https://github.com/arnakazim/TwiLiquidCrystal-library
#include <TwiLiquidCrystal.h>

// Create an LCD object and setting the I2C address
#define LCD_ADDRESS 0x3f
TwiLiquidCrystal lcd(LCD_ADDRESS);

String isEspResponseComplete = "";  // a String to hold incoming data
bool stringComplete = false;        // whether the string is complete
bool isLcdEnabled = false;          // Enable LCD
int initEspStep = 0;


void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial3.begin(115200);
  while (!Serial && !Serial3)
     ; // waiting for the serial monitor to become available
  // All set, we can proceed.
  Serial.println("\nBeer controller starting");
  isEspResponseComplete.reserve(128);
  initLcd();
}

void executeEspSteps() {
  switch (initEspStep) {
    case 0:
      Serial.println("Step 1");
      Serial3.write("AT\n");
      initEspStep ++;
      delay(500);
      break;
    case 1:
      Serial.println("Step 2");
      Serial3.write("AT+GMR\n");
      initEspStep ++;
      delay(500);
      break;
    case 2:
      // nothing else to do.
      break;
    default:
      Serial.println("Not a valid step.");
  }
  Serial3.flush();
}

void initLcd() {
  if (isLcdEnabled) {
    lcd.clear();
    lcd.home();
    lcd.backlight();
  }
}

/*
  This routine is run between each time loop() runs, so using delay inside 
  loop can delay response. Multiple bytes of data may be available.
*/
void evaluateESP() {
  while (Serial3.available()) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the isEspResponseComplete:
    isEspResponseComplete += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void loop() {
  // print the string when a newline arrives:
  executeEspSteps();
  evaluateESP();
  if (stringComplete) {
    Serial.println(isEspResponseComplete);
    // clear the string:
    isEspResponseComplete = "";
    stringComplete = false;
  }
// // Update and send command to other serial
//  if (Serial.available()) {
//    char resp = (char)Serial.read();
//    Serial3.write(resp);
//  }
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
