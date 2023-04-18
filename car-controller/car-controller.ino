/****************************************************************************************************************************
Based on https://github.com/khoih-prog/WebSockets2_Generic/blob/master/examples/Generic/WiFiNINA/SAMD/SAMD-Server/SAMD-Server.ino

*****************************************************************************************************************************/

#include "defines.h"
#include "LiquidCrystal_I2C.h"

#include <WebSockets2_Generic.h>

using namespace websockets2_generic;

#define LCD_NPN_PIN 6

WebsocketsServer server;
WebsocketsClient client;
LiquidCrystal_I2C lcd(0x3f, 16, 2);
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

void check_status()
{
  static unsigned long checkstatus_timeout = 0;

  //KH
#define HEARTBEAT_INTERVAL    10000L
  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = millis() + HEARTBEAT_INTERVAL;
  }
}

void setup()
{
  pinMode(LCD_NPN_PIN, INPUT);
  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  Serial.println("Please enable LCD...");
  while(!digitalRead(LCD_NPN_PIN)){}
  Serial.println("LCD power high");
  delay(250);

  lcd.begin(); // set up the lcd
  
  Serial.println("\nStarting car-controller with WiFiNINA on " + String(BOARD_NAME));
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

  lcd.clear();
  lcd.print("Starting up...");
  lcd.setCursor(0, 1);
  lcd.print("WiFiNINA v" + fv);
  delay(2000);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  lcd.clear();
  lcd.print("Connecting to:");
  lcd.setCursor(0, 1);
  lcd.print(String(ssid));

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
        lcd.clear();
        lcd.print("WiFi connection");
        lcd.setCursor(0, 1);
        lcd.print("failed!");
        while (1) {}
        break;
      default:
        Serial.print("Got other WiFi status: ");
        Serial.println(wiFiStatus);
        lcd.print("WiFi connection");
        lcd.setCursor(0, 1);
        lcd.print("failed!");
        break;
    }
  }
  Serial.println("\nWiFi connected");
  lcd.clear();
  lcd.print("Port: " + String(WEBSOCKETS_PORT) + " IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  server.listen(WEBSOCKETS_PORT);
  
  Serial.print(server.available() ? "WebSockets Server Running and Ready on " : "Server Not Running on ");
  Serial.println(BOARD_NAME);
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());     //You can get IP address assigned to SAMD
  Serial.print(", Port: ");
  Serial.println(WEBSOCKETS_PORT);    // Websockets Server Port
}

void loop()
{ 
  check_status();
  
  client = server.accept();
 
  while (client.available())
  {
    WebsocketsMessage msg = client.readNonBlocking();
    if(!msg.isEmpty()){
    // log
    Serial.print("Got Message: ");
    Serial.println(msg.data());

    // return echo
    client.send("Echo: " + msg.data());
    // parse message here
    }
    // do other stuff, this is an about 250-300Hz loop
  }

  delay(1000);
}
