#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThingsBoardHttp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "your ssid"; // this is wifi/Access Point SSID
const char* pass = "your password"; // this is the password for the wifi
const char* host = "thingsboard.cloud"; //  iot platform host
const char* tburl = "your thingboard url";
const char* devname = "your devname";
const char* devtype = "your devtype";
const char* devmodel = "your devmodel";

float voltage1, current1, power1, energy1, frequency1;
LiquidCrystal_I2C lcd(0x27, 20, 4);
PZEM004Tv30 pzem1(14, 12); // GPIO14(D5) to Tx PZEM004; GPIO12(D6) to Rx PZEM004

void connectWiFi() {
  int i=0;
  Serial.println("Connecting to WIFI");
  WiFi.begin(ssid, pass);
  while((!(WiFi.status() == WL_CONNECTED)))
  {
   Serial.print("."); 
    i++;
    delay(300);
    if( i>10 )    
    { 
     connectWiFi();
    }
   }  
   Serial.println("WiFi connected");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();                                                                                                                
  delay(1000);
  connectWiFi(); 
  
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("Power Meter");
  delay(1000);

}


void loop() {

  //THINGSBOARD
      while((!(WiFi.status() == WL_CONNECTED))) // if wifi status NOT connected, connect to wifi
    {
      connectWiFi();
      }  
    WiFiClient client; 
    const int httpPort = 443;
    if (!client.connect(host, httpPort)) // check if connected to ifttt server/host
      {  
         Serial.println("Connection to iot host failed");
         lcd.setCursor(0, 0);
         lcd.clear();
         lcd.print("NoCon"); 
         return;
      }
HTTPClient http; 

  voltage1 = pzem1.voltage();
  voltage1 = zeroIfNan(voltage1);
  current1 = pzem1.current();
  current1 = zeroIfNan(current1);
  power1 = voltage1 * current1;
  power1 = zeroIfNan(power1);
  energy1 = pzem1.energy(); //kwh
  energy1 = zeroIfNan(energy1);
  frequency1 = pzem1.frequency();
  frequency1 = zeroIfNan(frequency1);


  delay(1000);

  Serial.println("");
  Serial.printf("Voltage        : %.2f\ V\n", voltage1);
  Serial.printf("Current        : %.2f\ A\n", current1);
  Serial.printf("Power Active   : %.2f\ W\n", power1);
  Serial.printf("Frequency      : %.2f\ Hz\n", frequency1);
  Serial.printf("Energy         : %.2f\ kWh\n", energy1);
  Serial.printf("---------- END ----------");
  Serial.println("");

  lcd.setCursor(0, 0);
  lcd.printf("Voltage  : %.2f\ V", voltage1);
  lcd.setCursor(0, 1);
  lcd.printf("Current  : %.2f\ A", current1);
  lcd.setCursor(0, 2);
  lcd.printf("Frequency: %.2f\ Hz", frequency1);
  lcd.setCursor(0, 3);
  lcd.printf("Energy   : %.2f\ kWh", energy1);
  delay(1000);

  //send data to thingsboard (Json format)
  http.begin (client, tburl);
  http.addHeader ("Content-Type","application/json");
  StaticJsonDocument<200> doc;
  doc["deviceName"] = devname;
  doc["deviceType"] = devtype;
  doc["model"] = devmodel;
  doc["volt"] = voltage1;
  doc["cur"] = current1;
  doc["pow"] = power1;
  doc["en"] = energy1;
  doc["freq"] = frequency1;
  String reqbody;
  serializeJson(doc,reqbody);
  int httpResponseCode = http.POST(reqbody);
  if(httpResponseCode=0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    lcd.setCursor(0,0);
    //lcd.clear();
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
    //lcd.print("Data Sent");
    Serial.println("Data Sent");
  }
  http.end();


}

void printValue(String label, float value) {
  if (value != NAN) {
    Serial.print(label); Serial.println(value);
  } else {
    Serial.println("Error reading");
  }
}

float zeroIfNan(float v) {
  if (isnan(v)) {
    v = 0;
  }
  return v;
}
