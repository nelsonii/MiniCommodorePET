//
// ESP8266_OLED_GREENHOUSE_CLIENT_V3 -- For Greenhouse
//

//Board: WeMos D1 R2 & Mini
//CPU Freq: 80 Mhz
//Flash Size: 4M (3M SPIFFS)
//Upload speed: 921600

// Display is 16 chars wide, 7 lines tall

//=================================================================================

// Board's WIFI stack
#include <ESP8266WiFi.h>

//Display
#include <U8g2lib.h>

// Real Time Clock
#include <RTClib.h> // real time clock -- Code will work okay w/o clock

//=================================================================================

const char* ssid = "PONCE_2G";
const char* password = "TutPon692";
const char* host = "192.168.13.210"; // Fixed IP on our router, by MAC

// Turns on/off Serial Output debugging
const boolean DEBUG = 1;
const int BAUD = 115200;

// Code version. Used for developer reference.
const long APP_VER = 2018111501;
const String APP_NAME = "ESP8266_OLED_GREENHOUSE_CLIENT_V3";

//=================================================================================

// Create instance of display driver
U8G2_SH1106_128X64_NONAME_1_HW_I2C display (U8G2_R0); //No rotation, landscape

// Real Time Clock
RTC_DS1307 rtc; // real time clock

//=================================================================================

void setup() {

  if (DEBUG) {
    Serial.begin(BAUD); 
    delay(100); 
    Serial.println("********* SETUP START *********"); 
    Serial.println(APP_NAME); 
    Serial.println(APP_VER);
    }

  //tone(5, 1000, 2000); //pin,freq,dur
  
  if (DEBUG) {Serial.print("Connecting WiFi To: ");Serial.println(ssid);}

  WiFi.hostname("COM PET");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    if (DEBUG) {Serial.print("."); delay(500);}
  }

  if (DEBUG) {Serial.println("WiFi connected @ " + WiFi.localIP());}

  // Start real time clock
  if (DEBUG) {Serial.println("Starting Real Time Clock...");}
  if (! rtc.begin()) {
    if (DEBUG) {Serial.println("Real Time Clock not found!");}
    }

  // If clock isn't running (set to 0:0:0) then adjust the date/time on RTC to compile date/time.
  if (! rtc.isrunning()) {
    if (DEBUG) {Serial.println("RTC not running. Adjusting to Compile Date/Time");}
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // Reset clock based on compile date & time
  if (rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  //Start up the OLED display
  display.begin();

  if (DEBUG) {Serial.println("********* SETUP DONE *********");}
  if (DEBUG) {Serial.println("Starting Main...");}

}//setup

//=================================================================================

void loop() {

  // Check if a client has connected
  WiFiClient client;

  if (!client.connect(host, 80)) {
    if (DEBUG) {Serial.print("Can't Connect to ");Serial.println(host);}
    delay(10000); // wait 10 seconds
    return;
  }

    client.print(String("GET /") + " HTTP/1.1\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        if (DEBUG) {Serial.print("No Response from "); Serial.println(host);}
        client.stop();
        delay(10000); // wait 10 seconds
        return;
    }
   }    

    // Real time clock
    DateTime now = rtc.now();
    String timeStamp = (String) now.year() + "/" + (String) now.month() + "/" + (String) now.day() + " " + (String) now.hour() + ":" + (String) now.minute() + ":" +(String) now.second() ;
    String displayTime = (String) now.hour() + ":" + (String) now.minute();

  // If we reach this point we are connected and received a response from the server
   
    bool isPayload = false; // Used to determine a section of the response we care about
    String payload = "";
    
    while (client.available())
      {
        char ch = static_cast<char>(client.read());
        if (ch == '{') { isPayload = true; }
        if (ch == '}') { isPayload = false; }
        if (isPayload) {if (ch != '{') {payload.concat(ch);}}
      }
    client.stop();
    
    payload.concat("|"); // add trailing pipe
    
    if (DEBUG) {Serial.print(timeStamp);Serial.print("  -  ");Serial.println(payload);}

    display.firstPage();
    /* all graphics commands have to appear within the loop body. */    
    do {
      //getValue position 0=IP, 1=Temp, 2=Pres, 3=Elev
      //the outer getValue looking for '.' is a hack to strip the decimal values from temp. 
      String sensorTemp = getValue(getValue(payload, '|', 1), '.', 0) + " F";
      
      display.setFont(u8g2_font_fur42_tr);
      display.setCursor(0,50);
      display.print(sensorTemp);
      
    } while ( display.nextPage() );

    delay(60000); // execute once every minute

    //If the code hangs (can't connect to remote server)
    //the user will see the dashes on the screen
    display.firstPage();
    do {
      display.setFont(u8g2_font_fur42_tr);
      display.setCursor(0,50);
      display.print("N/A");
    }while (display.nextPage());

}//loop

//=================================================================================

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}//getValue
