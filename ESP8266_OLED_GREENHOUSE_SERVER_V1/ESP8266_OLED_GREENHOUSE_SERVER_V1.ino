//
// ESP8266_OLED_GREENHOUSE_SERVER_V1 -- For Greenhouse
//

//Board: WeMos D1 R2 & Mini
//CPU Freq: 80 Mhz
//Flash Size: 4M (3M SPIFFS)
//Upload speed: 921600

// Display is 16 chars wide, 7 lines tall


// Barometric sensor
#include <Adafruit_BMP085.h>

// Board's WIFI stack
#include <ESP8266WiFi.h>

//Display
#include <U8x8lib.h>

const char* ssid = "PONCE_2G";
const char* password = "TutPon692";

const uint8_t colLow = 4;
const uint8_t colHigh = 13;

// Barometric Class
Adafruit_BMP085 bmp;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

// Create instance of display driver
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);


void setup() {

  // Start Barometric Sensor
  bmp.begin();
  
  u8x8.begin();
  u8x8.setFont(u8x8_font_victoriamedium8_r); // NORMAL
  u8x8.clear();

  u8x8.setCursor(0,1);
  u8x8.print("Connecting to");
  u8x8.setCursor(0,2);
  u8x8.print(ssid);

  WiFi.hostname("GREENHOUSE");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    u8x8.print(".");
    delay(500);
  }
  u8x8.setCursor(0,3);
  u8x8.print("WiFi connected");
  
  // Start the server
  server.begin();
  u8x8.setCursor(0,5);
  u8x8.print("Server started @");

  // Print the IP address
  u8x8.setCursor(0,6);
  u8x8.print(WiFi.localIP());
}

void loop() {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  u8x8.clear();
  u8x8.setCursor(0,1);
  u8x8.print("   Server IP:   ");
  u8x8.setCursor(0,2);
  u8x8.print(WiFi.localIP());
  u8x8.setCursor(0,3);
  u8x8.print("   Client IP:   ");
  u8x8.setCursor(0,4);
  u8x8.print(client.remoteIP());
  
  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  //u8x8.print(req);
  client.flush();


  String temp = String((bmp.readTemperature() * 9/5) + 32);
  String pres = String(bmp.readPressure()  * 0.00029530);
  String alti = String(bmp.readAltitude() * 3.281);
  String svip = WiFi.localIP().toString();

  u8x8.setCursor(0,5);
  u8x8.print("Temp:" + temp + " F");
  u8x8.setCursor(0,6);
  u8x8.print("Pres:" + pres + " inHg");
  u8x8.setCursor(0,7);
  u8x8.print("Alti:" + alti + " ft");

  client.flush(); // ? why another flush ?
  
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<H1>Welcome to the Greenhouse.<BR>";
  s += "IP:" + svip + " <BR><BR>";
  s += "Temperature:" + temp + " F.<BR>";
  s += "Pressure:" + pres + " inHg.<BR>"; 
  s += "Altitude:" + alti + " ft.<BR><BR></H1>"; 
  s += "{" + svip + "|" + temp + "|" + pres + "|" + alti + "}";
  s += "</html>\n";

   // Send the response to the client
  client.print(s);
  delay(1);

  client.stop(); // testing this
  
}
