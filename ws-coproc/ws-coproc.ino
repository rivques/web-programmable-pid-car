/****************************************************************************************************************************
Based on https://github.com/khoih-prog/WebSockets2_Generic/blob/master/examples/Generic/WiFiNINA/SAMD/SAMD-Server/SAMD-Server.ino
and therefore licenced under GPL v3.0
 *****************************************************************************************************************************/

#include "defines.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <PID_v1.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SYSRESETREQ    (1<<2)
#define VECTKEY        (0x05fa0000UL)
#define VECTKEY_MASK   (0x0000ffffUL)
#define AIRCR          (*(uint32_t*)0xe000ed0cUL) // fixed arch-defined address
#define REQUEST_EXTERNAL_RESET (AIRCR=(AIRCR&VECTKEY_MASK)|VECTKEY|SYSRESETREQ)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define CONNECTED_PIN 10
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <WebSockets2_Generic.h>

using namespace websockets2_generic;

WebsocketsServer server;
WebsocketsClient client;

bool lastConnected = false;

void heartBeatPrint()
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("H"));        // H means server WiFi connected
  else  
    Serial.print(F("F"));        // F means server WiFi not connected
    
  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void reset() {
  REQUEST_EXTERNAL_RESET;
  while(1){}
}

void setup()
{
  Serial.begin(115200); // debug over USB
  Serial1.begin(115200); // WS over pins 0 and 1
  Serial1.setTimeout(50); // enough time for about 720 bytes
  while (!Serial && millis() < 5000);
  
  pinMode(CONNECTED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(CONNECTED_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  // Clear the buffer
  

  //lcd.begin(16, 2); // set up the lcd

  Serial.println("\nStarting SAMD-Server with WiFiNINA on " + String(BOARD_NAME));
  Serial.println(WEBSOCKETS2_GENERIC_VERSION);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println(F("Communication with WiFi module failed!"));
    // don't continue
    return;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) 
  {
    Serial.println(F("Please upgrade the firmware"));
  }

  display.setTextColor(SSD1306_WHITE);
  display.println("WiFiNINA version: ");
  display.setTextSize(2);
  display.println(fv);
  display.setTextSize(1);
  display.println("Connecting to SSID: ");
  display.setTextSize(2);
  display.println(ssid);
  display.setTextSize(1);

  display.display();

  Serial.print(F("Attempting to connect to SSID: "));
  Serial.println(ssid);

  // Connect to wifi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    char wiFiStatus = WiFi.status();
    switch(wiFiStatus){
      case WL_IDLE_STATUS:
        Serial.println(F("Connecting to Wifi..."));
        break;
      case WL_CONNECT_FAILED:
        Serial.println(F("Failed to connect to WiFi!"));
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("WiFi connection failed, resetting!");
        display.display();
        reset();
        break;
      case 6:
        // not sure why this happens but resetting usually fixes it
        Serial.println(F("Failed to connect to WiFi!"));
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("WiFi connection failed, resetting!");
        display.display();
        reset();
        break;
      default:
        Serial.print(F("Got other WiFi status: "));
        Serial.println((int)wiFiStatus);
        break;
    }
  }
  Serial.println(F("\nWiFi connected"));

  server.listen(WEBSOCKETS_PORT);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Port: ");
  display.println(String(WEBSOCKETS_PORT));
  display.println("IP Addr:");
  display.print(WiFi.localIP());
  display.display();
  display.setTextSize(1);
  
  Serial.print(server.available() ? "WebSockets Server Running and Ready on " : "Server Not Running on ");
  Serial.println(BOARD_NAME);
  Serial.print(F("IP address: "));
  Serial.print(WiFi.localIP());     //You can get IP address assigned to SAMD
  Serial.print(F(", Port: "));
  Serial.println(WEBSOCKETS_PORT);    // Websockets Server Port
}

void loop()
{ 
  client = server.accept();
 
  while (client.available())
  {
    if(!lastConnected){
      lastConnected = true;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("Connected");
      display.display();
      display.setTextSize(1);

      digitalWrite(CONNECTED_PIN, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      
      //char buffer[50];
      //sprintf(buffer, "G:%s,%s,%s,%s", String(kP).c_str(), String(kI).c_str(), String(kD).c_str(), String(setpoint).c_str());
      //Serial.println(buffer);
      //client.send(buffer);
      //Serial.println(F("Sent gains to client"));
    }
    WebsocketsMessage msg = client.readNonBlocking();
    if(!msg.isEmpty()){
      // log
      Serial.print(F("Got Message: "));
      Serial.println(msg.data());
      Serial1.println(msg.data()); // send to main processor
      // return echo
      //client.send("Echo: " + msg.data());
      //carPID.SetTunings(kP, kI, kD);
    } else {
      //Serial.println(F("No message waiting"));
    }
    if(Serial1.available()){
      String dataToSend = Serial1.readStringUntil('\n');
      client.send(dataToSend);
    }

  }
  if(lastConnected){
    Serial.println(F("Just dc'd from client"));
    lastConnected = false;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Port: ");
    display.println(String(WEBSOCKETS_PORT));
    display.println("IP Addr:");
    display.print(WiFi.localIP());
    display.display();
    display.setTextSize(1);
    digitalWrite(CONNECTED_PIN, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
