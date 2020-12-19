#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "MAX30100_PulseOximeter.h"
#include "ThingSpeak.h"

#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>

#define REPORTING_PERIOD_MS     1000
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
PulseOximeter pox;

float A = -60;
float N = 3;
float Distance = 0.0;
uint32_t tsLastReport = 0;
const char* ssid_= "GUARDIAN";
const char* pass_="12345678";
const char* ssid= "realme 3 Pro";
const char* pass="khusi123";
char auth[] = "WB_VLtGeK5v8-VhBcXKNnH_jmtk65nMt"; 
unsigned long channel_number = 1256261;            // Channel ID
const char * write_api_key = "UHMHNH3C4R8QRMEE"; // Read API Key

WiFiClient  client;
void onBeatDetected()
{
    Serial.println("Beat Detected!");
}
 
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid_,pass_);
  WiFi.disconnect();
  Blynk.begin(auth, ssid, pass);
  if (!pox.begin()) {
        Serial.println("FAILED OXIMETER");
        for(;;);
    } else {
        Serial.println("SUCCESS OXIMETER");
        
    }
   pox.setOnBeatDetectedCallback(onBeatDetected);
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // Clear the buffer.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  Serial.println("Connecting to Wifi");
 WiFi.begin(ssid,pass);
while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("Wifi Connected");
  ThingSpeak.begin(client);
  display.setFont();
  display.setTextSize(1);
  display.clearDisplay();
  // Greetings!
  display.setTextColor(WHITE);
  display.setCursor(5,25);
  display.println("Presenting to you");
  
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(5,50);
  display.println("GUARDIAN!");
  delay(5000);
  display.display();
}

void loop() {
  // WiFi.scanNetworks will return the number of networks found
 
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      float ratio = (A - WiFi.RSSI(i))/(10 * N);
      Distance = pow(10,ratio);
      Serial.println(Distance);
      if(Distance<1){
          digitalWrite(LED_BUILTIN, HIGH);   
          delay(500);                      
          digitalWrite(LED_BUILTIN, LOW);    
          delay(500);  }
      delay(10);
    }
    
    pox.update();
    Blynk.run();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        float heart_rate = random(100);
        float o2= random(100);       
        display.clearDisplay();
          //Heart symbol
        display.setFont();
        display.setCursor(5,3);
        display.println("Heart rate and");
        display.setCursor(5,12);
        display.println("oxygen level");
        display.setCursor(5,20);
        display.setTextSize(3);
        display.write(3);

        //Heart rate
        display.setTextSize(2);
        display.setCursor(30,25);
        display.print(heart_rate);  

        //O2 symbol
        display.setCursor(5,50);
        display.println("O2: ");
        //O2 level
        display.setCursor(30,50);
        display.print(o2); 
        display.display();
        display.setFont();
        display.setTextSize(1);
        display.clearDisplay();
        Blynk.virtualWrite(V0, heart_rate);
        Blynk.virtualWrite(V1, o2);
        ThingSpeak.setField( 1, heart_rate);
        ThingSpeak.setField( 2, o2);
        ThingSpeak.writeFields(channel_number,write_api_key);
        tsLastReport = millis();
    }
  
  Serial.println("");

  // Wait a bit before scanning again
  delay(1000);
}
