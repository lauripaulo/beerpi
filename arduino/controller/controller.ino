#include <stdio.h>
#include <Wire.h>
#include <TwiLiquidCrystal.h>    // Arnakazim's TwiLiquidCrystal Arduino Library
#include <OneWire.h> 
#include <DallasTemperature.h>

#define LCD_ADDRESS   0x3f       // Create an LCD object and setting the I2C address
#define ONE_WIRE_BUS  2          // Data wire is plugged into pin 2 on the Arduino 

const int TIMEOUT = 10000;

const int STATE_IDLE            = 1;
const int STATE_ERROR           = 2;
const int STATE_PROBING_ESP8266 = 3;
const int STATE_WIFI_CONECT     = 4;
const int STATE_GETIP           = 5;
const int STATE_START_SERVER    = 6;
const int STATE_SEND            = 7;
const int STATE_DISABLE_SEND    = 8;

const char* ESP_TEST         = "AT";                  // This will check if the module is connected properly and its functioning, the module will reply with an acknowledgment.
const char* ESP_RESET        = "AT+RST";              // This will reset the wifi module. Its good practice to reset it before or after it has been programmed.
const char* ESP_INFO         = "AT+GMR";              // This will mention the firmware version installed on the ESP8266.
const char* ESP_LIST         = "AT+CWLAP";            // This will detect the Access points and their signal strengths available in the area.
const char* ESP_CONNECT      = "AT+CWJAP=";           // AT+CWJAP=”SSID”,”PASSWORD” This connects the ESP8266 to the specified SSID in the AT command mentioned in the previous code.
const char* ESP_IP           = "AT+CIFSR";            // This will display the ESP8266’s obtained IP address.
const char* ESP_DISCONECT    = "AT+CWJAP=\"\",\"\"";  // If the user wants to disconnect from any access point.
const char* ESP_SETWIFI      = "AT+CWMODE=1";         // This sets the Wifi mode. It should be always set to Mode 1 if the module is going to be used as a node
const char* ESP_SEND_ENABLE  = "AT+CIPMODE=1";        // Enable UART-WiFi passthrough mode. 
const char* ESP_MULTICONN    = "AT+CIPMUX=1";         // Enable multiple connections, 1 == enabled.

/*
Create a TCP server, default	port	=	333
When ESP8266 received data from server, it will prompt message below:

+IPD,0,n:xxxxxxxxxx				//	received	n	bytes,		data=xxxxxxxxxxx
*/
const char* ESP_STARTSERVER  = "AT+CIPSERVER=1";

/*
Send data, example:

AT+CIPSEND=0,4				 //	set	data length which will be sent, such as 4 bytes
Recv 4	bytes

SEND OK
*/
const char* ESP_SEND_DATA   = "AT+CIPSEND=0,";

// Wifi info
const char* WIFI_SSID     = "BaenaLaux-2.4G-(Atico)";
const char* WIFI_PASSWD   = "100%WifiNet";

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 

const TwiLiquidCrystal lcd(LCD_ADDRESS);

struct Therms {
  int t1;
  int t2;
  int t3;
};

/* ***
   *** Global vars - Take extra care! ***
   ***
*/
bool globalUartReadComplete = false;    // whether the string is complete
byte globalState = STATE_IDLE;
byte lastGlobalState = globalState;
String globalUartBuffer = "";
String globalIpAddress = "";
Therms globalTemps = {0, 0, 0};
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature.

void readTemps()
{
  globalTemps.t1 = 1.1;
  globalTemps.t2 = 2.2;
  globalTemps.t2 = 3.3;
}

String getIpAddress() {
  return globalIpAddress;
}

void setIpAddress(String address) {
  globalIpAddress = address;
}

String getUartBuffer() {
  return globalUartBuffer;
}

void setUartBuffer(String buffer) {
  globalUartBuffer = buffer;
}

byte getLastGlobalState(byte state) {
  return lastGlobalState;
}

void changeGlobalState(byte state) {
  lastGlobalState = globalState;
  globalState = state;
  char buffer[64];
  char lcdBuffer[8];
  sprintf(buffer, "> changeGlobalState:%02i / lastGlobalState:%02i", globalState, lastGlobalState);
  Serial.println(buffer);
  sprintf(lcdBuffer, "%02i/%02i", globalState, lastGlobalState);
  lcd.setCursor(15, 0);
  lcd.print(lcdBuffer);
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
  lcd.begin(20,4);
  lcd.clear();
  lcd.home();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("BeerDuino 2022");
  Serial.println("Dallas Temperature IC Control Library Demo");  
  sensors.begin();    // Start up the library
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
  bool completed = false;
  delay(100); // give some time to esp8266 work between commands.
  unsigned long startTime = millis();
  Serial.print("UART:\n---\n");
  resetUartReadState();
  Serial3.println(cmd);
  do {
    readUART();
    if (millis() - startTime > timeout) {
      Serial.println("UART timeout!");
      Serial3.flush();
      return false;
    }
    if (getUartBuffer().lastIndexOf("ERROR") > 0) {
      Serial3.flush();
      return false;
    }
    completed = getUartBuffer().lastIndexOf("OK") > 0;
  } while (!completed);
  Serial.print(getUartBuffer());
  Serial.println("---");
  return true;
}


void loop() {
  if (getGlobalState() == STATE_PROBING_ESP8266) {
    Serial.println("Testing ESP8266 conectivity...");
    if (execEsp8266Cmd(ESP_TEST, TIMEOUT)) {
      Serial.println("ESP8266 found at Serial3 (UART).");
      changeGlobalState(STATE_WIFI_CONECT);
    } else {
      Serial.println("Error, ESP8266 not found.");
      changeGlobalState(STATE_ERROR);
    }
  }

  if (getGlobalState() ==  STATE_WIFI_CONECT) {
    Serial.println("Waiting wifi...");
    String connectCmd = String(ESP_CONNECT) + "\"" + WIFI_SSID + "\",\"" + WIFI_PASSWD + "\"";
    if (execEsp8266Cmd(connectCmd.c_str(), TIMEOUT)) {
      Serial.println("Wifi connected!");
      changeGlobalState(STATE_GETIP);
    } else {
      Serial.println("Cannot connect to Wifi.");
      changeGlobalState(STATE_ERROR);
    }
  }

  if (getGlobalState() ==  STATE_GETIP) {
    delay(500); // give esp8266 time to think.
    if (execEsp8266Cmd(ESP_IP, TIMEOUT)) {
      int start = getUartBuffer().indexOf("STAIP") + 7;
      int end = getUartBuffer().lastIndexOf("+CIFSR:STAMAC") - 3;
      setIpAddress(getUartBuffer().substring(start, end));
      changeGlobalState(STATE_START_SERVER);
    } else {
      changeGlobalState(STATE_ERROR);
    }
  }

  if (getGlobalState() == STATE_START_SERVER) {
    if(execEsp8266Cmd(ESP_MULTICONN, TIMEOUT)) {
      if (execEsp8266Cmd(ESP_STARTSERVER, TIMEOUT)) {
        Serial.print("Server started! IP=");
        Serial.println(getIpAddress());
        lcd.setCursor(0, 1);
        lcd.print(getIpAddress());
        resetUartReadState();
        changeGlobalState(STATE_IDLE);
      } else {
        changeGlobalState(STATE_ERROR);
      }
    } else {
      changeGlobalState(STATE_ERROR);
    }
  }

  if (getGlobalState() == STATE_IDLE) {
    readTemps();
    readUART();
    if (isUartReadComplete()) {
      char msg [32];
      char cmd [16];
      Serial.print("Received:\n---\n");
      Serial.print(getUartBuffer());
      Serial.println("\n---");
      // commands
      if (getUartBuffer().indexOf("IPD" > 0)) {
        String recv = getUartBuffer().substring(getUartBuffer().indexOf(":") + 1, getUartBuffer().length());
        resetUartReadState();
        if(recv == "temps") {
          sprintf(msg, "t1=%2.2f;t2%2.2f;t3=%2.2f", globalTemps.t1, globalTemps.t2, globalTemps.t3);
          sprintf(cmd, "%s%d", ESP_SEND_DATA, strlen(msg));
          sendToClient(cmd, msg);
        } else if (recv == "state") {
          sprintf(msg, "state=%d;lastState=%d", globalState, lastGlobalState);
          sprintf(cmd, "%s%d", ESP_SEND_DATA, strlen(msg));
          sendToClient(cmd, msg);
        } else {
          Serial.print(">>> recv: ");
          Serial.println(recv);
          sendToClient(ESP_SEND_DATA, "nok");
        }
      }
    }
  }

  if (getGlobalState() == STATE_ERROR) {
    Serial.println("Fatal ERROR. System halted!");
    delay(5000);  
  }

}

void sendToClient(char cmd[], char msg[]) {
  if (execEsp8266Cmd(cmd, TIMEOUT)) {
    resetUartReadState();
    if (execEsp8266Cmd(msg, TIMEOUT)) {
      Serial.println(msg);
      Serial3.flush();
      // add timeout to this loop.
      while (true) {
        readUART();
        if (isUartReadComplete()) {
          if (getUartBuffer().indexOf("SEND OK") >=0 ) {
            resetUartReadState();
            break;
          }
        }
      }
    }
  }
}

// void loop() {
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
// }

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
