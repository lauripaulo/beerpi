#include <Wire.h>
#include <TwiLiquidCrystal.h>                     // Arnakazim's TwiLiquidCrystal Arduino Library

// Create an LCD object and setting the I2C address
#define LCD_ADDRESS 0x3f

#define STATE_IDLE              1
#define STATE_ERROR             2
#define STATE_PROBING_ESP8266   3

#define TIMEOUT                 5000

const char* ESP_TEST      = "AT";                // This will check if the module is connected properly and its functioning, the module will reply with an acknowledgment.
const char* ESP_RESET     = "AT+RST";              // This will reset the wifi module. Its good practice to reset it before or after it has been programmed.
//String ESP_INFO      = "AT+GMR";              // This will mention the firmware version installed on the ESP8266.
//String ESP_LIST      = "AT+CWLAP";            // This will detect the Access points and their signal strengths available in the area.
//String ESP_CONNECT   = "AT+CWJAP=";           // AT+CWJAP=”SSID”,”PASSWORD” This connects the ESP8266 to the specified SSID in the AT command mentioned in the previous code.
//String ESP_IP        = "AT+CIFSR";            // This will display the ESP8266’s obtained IP address.
//String ESP_DISCONECT = "AT+CWJAP=\"\",\"\"";  // If the user wants to disconnect from any access point.
//String ESP_SETMODE   = "AT+CWMODE=1";         // This sets the Wifi mode. It should be always set to Mode 1 if the module is going to be used as a node
//
const TwiLiquidCrystal lcd(LCD_ADDRESS);

// String uartContent = "";         // a String to hold incoming data
bool globalUartReadComplete = false;    // whether the string is complete
bool isLcdEnabled = false;          // Enable LCD
byte globalState = STATE_IDLE;
String globalUartBuffer = "";

String getUartBuffer() {
  return globalUartBuffer;
}

void setUartBuffer(String buffer) {
  globalUartBuffer = buffer;
}

void changeGlobalState(byte state) {
  globalState = state;
}

byte getGlobalState() {
  return globalState;
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial3.begin(115200);
  while (!Serial && !Serial3)
    ;  // waiting for the serial monitor to become available
  // All set, we can proceed.
  changeGlobalState(STATE_PROBING_ESP8266);
  Serial.println("Beer controller starting");
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
void readUART() {
  while (Serial3.available()) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the uartContent:
    setUartBuffer(getUartBuffer() + inChar);
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      setUartReadComplete(true);
    }
  }
}

void resetUartReadState() {
  setUartReadComplete(false);
  setUartBuffer("");
}

bool isUartReadComplete() {
  return globalUartReadComplete;
}

void setUartReadComplete(bool completed) {
  globalUartReadComplete = completed;
}

bool execEsp8266Cmd(const char* cmd, unsigned long timeout) {
  bool res = false;
  bool completed = false;
  unsigned long startTime = millis();
  resetUartReadState();
  Serial3.println(cmd);
  do {
    delay(50); // give uart buffer time to think.
    readUART();
    if (millis() - startTime > timeout) {
      Serial.println("UART timeout!");
      return false;
    }
    completed = getUartBuffer().lastIndexOf("OK") > 0 || getUartBuffer().lastIndexOf("ERROR") > 0;
  } while (!completed);
  Serial.print("UART:\n---\n");
  Serial.print(getUartBuffer());
  Serial.println("---");
  if (getUartBuffer().lastIndexOf("OK") > 0) {
    res = true;
  }
  return res;
}


void loop() {
  switch (getGlobalState()) {
    case STATE_PROBING_ESP8266:
      Serial.println("Testing ESP8266 conectivity...");
      if (execEsp8266Cmd(ESP_TEST, TIMEOUT)) {
        Serial.println("ESP8266 found at Serial3 (UART).");
        changeGlobalState(STATE_IDLE);
      } else {
        Serial.println("Error, ESP8266 not found.");
        Serial.println("Fatal ERROR.");
        changeGlobalState(STATE_ERROR);
      }
      break;

    case STATE_ERROR:
      delay(5000);
      Serial.println("System halted!");
      break;

    case STATE_IDLE:
      delay(50); // nothing to do yet...
  }
}


//void loop() {
//  // print the string when a newline arrives:
//  readUART();
//  if (isUartReadComplete()) {
//    Serial.print(getUartBuffer());
//    // clear the string:
//    resetUartReadState();
//  }
//  // Update and send command to other serial
//  if (Serial.available()) {
//    char resp = (char)Serial.read();
//    Serial3.write(resp);
//  }
//}

/*
   Helper function to find first i2c address in the bus.
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
