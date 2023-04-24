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
#define TRIG_PIN 11
#define ECHO_PIN 12
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <WebSockets2_Generic.h>

using namespace websockets2_generic;

WebsocketsServer server;
WebsocketsClient client;

bool lastConnected = false;

double kP = 1;
double kI = 0.5;
double kD = 0.1;

// https://playground.arduino.cc/Code/PIDLibaryBasicExample/

double setpoint = 40;
double output = 0;
double distanceABCDEF = 0;

//PID carPID(&distance, &output, &setpoint, kP, kI, kD, DIRECT);

double getDistanceCm(){
  // Sets the trigPin on HIGH state for 10 micro seconds
  //digitalWrite(TRIG_PIN, HIGH);
  //delayMicroseconds(10);
  //delay(1);
  //digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  //unsigned long duration = pulseIn(ECHO_PIN, HIGH, 24000);
  //unsigned long duration = 200;
  // Calculating the distance
  //return duration * 0.034 / 2;
  return 30;
}

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
  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  //Serial.println("Please enable LCD...");
  //while(!digitalRead(LCD_NPN_PIN)){}
  //Serial.println("LCD power high");
  //delay(250);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  //carPID.SetMode(AUTOMATIC);
  //carPID.SetOutputLimits(-1, 1);
 

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
        display.print("WiFi connection failed, resetting!");
        display.display();
        reset();
        break;
      case 6:
        // not sure why this happens but resetting usually fixes it
        Serial.println("Got case 6 when connecting to WiFi, resetting...");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("WiFi connection case 6, resetting...");
        display.display();
        //delay(2000);
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
    if(!lastConnected){
      lastConnected = true;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("Connected");
      display.display();
      display.setTextSize(1);
      
      char buffer[50];
      sprintf(buffer, "G:%s,%s,%s,%s", String(kP).c_str(), String(kI).c_str(), String(kD).c_str(), String(setpoint).c_str());
      Serial.println(buffer);
      client.send(buffer);
      Serial.println("Sent gains to client");
    }
    WebsocketsMessage msg = client.readNonBlocking();
    if(!msg.isEmpty()){
      // log
      Serial.print("Got Message: ");
      Serial.println(msg.data());
      // parse message
      char newP[16], newI[16], newD[16], newS[16];
      sscanf(msg.c_str(), "%s %s %s %s", newP, newI, newD, newS);
      kP = atof(newP);
      kI = atof(newI);
      kD = atof(newD);
      setpoint = atof(newS);
      Serial.print("Gains now kP: ");
      Serial.print(kP);
      Serial.print(" kI: ");
      Serial.print(kI);
      Serial.print(" kD: ");
      Serial.println(kD);
      Serial.print("Setpoint: ");
      Serial.println(setpoint);
      // return echo
      //client.send("Echo: " + msg.data());
      //carPID.SetTunings(kP, kI, kD);
    } else {
      Serial.println("No message waiting");
    }
    // do other stuff, loops at around 300Hz
    //distance = getDistanceCm();
    distanceABCDEF = 0;
    //if(distance == 0){
      //distance = 400; // getDistanceCm is 0 when out of range
    //}
    //carPID.Compute();
    // TODO: output
    //char buffer[50];
    //sprintf(buffer, "F:%s,%s,%s,%s,%s", String(distance).c_str(), String(setpoint).c_str(), String(distance-setpoint).c_str(), String(output), String(millis()));
    //client.send(buffer);
    Serial.println("connected loop");
  }
  if(lastConnected){
    Serial.println("Just dc'd from client");
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
  }
}
