/****************************************************************************************************************************
  SAMD-Server.ino
  For SAMD21/SAMD51 with WiFiNINA module/shield.
  
  Based on and modified from Gil Maimon's ArduinoWebsockets library https://github.com/gilmaimon/ArduinoWebsockets
  to support STM32F/L/H/G/WB/MP1, nRF52 and SAMD21/SAMD51 boards besides ESP8266 and ESP32
  
  The library provides simple and easy interface for websockets (Client and Server).
  
  Example first created on: 10.05.2018
  Original Author: Markus Sattler
  
  Built by Khoi Hoang https://github.com/khoih-prog/Websockets2_Generic
  Licensed under MIT license
 *****************************************************************************************************************************/
/****************************************************************************************************************************
  SAMD Websockets Server : Minimal SAMD21/SAMD51 Websockets Server

  This sketch:
        1. Connects to a WiFi network
        2. Starts a websocket server on port 8080
        3. Waits for connections
        4. Once a client connects, it wait for a message from the client
        5. Sends an "echo" message to the client
        6. closes the connection and goes back to step 3

  Hardware:
        For this sketch you only need a SAMD21/SAMD51 board.

  Originally Created  : 15/02/2019
  Original Author     : By Gil Maimon
  Original Repository : https://github.com/gilmaimon/ArduinoWebsockets

*****************************************************************************************************************************/

#include "defines.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <core_cm4.h>

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
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <WebSockets2_Generic.h>

using namespace websockets2_generic;

WebsocketsServer server;
WebsocketsClient client;

bool justDisconnected = false;

void heartBeatPrint()
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print("H");        // H means server WiFi connected
  else  
    Serial.print("F");        // F means server WiFi not connected
    
  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(" ");
  }
}

void reset() {
  REQUEST_EXTERNAL_RESET;
  while(1){}
}

void setup()
{
  //pinMode(LCD_NPN_PIN, INPUT);
  //Serial.begin(115200);
  while (!Serial && millis() < 5000);
  //Serial.println("Please enable LCD...");
  //while(!digitalRead(LCD_NPN_PIN)){}
  //Serial.println("LCD power high");
  //delay(250);

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
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    return;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) 
  {
    Serial.println("Please upgrade the firmware");
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

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++)
  {
    Serial.print(".");
    delay(1000);
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    char wiFiStatus = WiFi.status();
    switch(wiFiStatus){
      case WL_IDLE_STATUS:
        Serial.println("Connecting to Wifi...");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("Failed to connect to WiFi!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("WiFi connection failed, please check serial!");
        display.display();
        while (1) {}
        break;
      case 6:
        // not sure why this happens but resetting usually fixes it
        Serial.println("Got case 6 when connecting to WiFi, resetting...");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("WiFi connection case 6, resetting...");
        display.display();
        delay(2000);
        reset();
        break;
      default:
        Serial.print("Got other WiFi status: ");
        Serial.println((int)wiFiStatus);
        break;
    }
  }
  Serial.println("\nWiFi connected");

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
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());     //You can get IP address assigned to SAMD
  Serial.print(", Port: ");
  Serial.println(WEBSOCKETS_PORT);    // Websockets Server Port
}

void loop()
{ 
  client = server.accept();
 
  while (client.available())
  {
    if(!justDisconnected){
      justDisconnected = true;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("Connected");
      display.display();
      display.setTextSize(1);
    }
    WebsocketsMessage msg = client.readNonBlocking();
    if(!msg.isEmpty()){
      // log
      Serial.print("Got Message: ");
      Serial.println(msg.data());
  
      // return echo
      client.send("Echo: " + msg.data());
      // parse message here
    }
    // do other stuff, loops at around 300Hz
  }
  if(justDisconnected){
    justDisconnected = false;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Port: ");
    display.println(String(WEBSOCKETS_PORT));
    display.println("IP Addr:");
    display.print(WiFi.localIP());
    display.display();
    display.setTextSize(1);
  }
}
