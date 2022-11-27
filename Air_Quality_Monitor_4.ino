#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char auth[] = "ecoVceaxZG5QXT0xP2lqiZDc1csiL_Xj";
char ssid[] = "Đẹp thì vô";
char pass[] = "undercut";

#define DHTPIN         D5
#define DHTTYPE        DHT11
#define ONE_WIRE_BUS   D6

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature ds(&oneWire);
WidgetLED led1(V2);
WiFiClient espClient;
WiFiUDP udp;
NTPClient ntp(udp, "0.vn.pool.ntp.org", 7*3600);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

#define BLYNK_LIGHT_YELLOW    "#fffb00"
#define BLYNK_YELLOW          "#ED9D00"
#define BLYNK_ORANGE          "#fa5300"
#define BLYNK_RED             "#ff0000"

int heat_index[13][16] = {
  {0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3},
  {0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3},
  {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3},
  {0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3},
  {0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3},
  {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3},
  {0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
  {0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
};

byte thermometro[8] = //icon for termometer
{
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};


byte igrasia[8] = //icon for water droplet
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t clock1[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};

float t;
int h;
//float ppm;
int ionizer = D7;
int button = 1;
unsigned long timer0 = 0;






void read_sensor(){
  get_sensor();
  
  if(t<200 && h<=100){
    lcd_disp();
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);
    //Blynk.virtualWrite(V3, ppm/1023*100);
  }

  int i = (h-40)/5;
  int j = t/27;
  int a = heat_index[i][j];
  if(a == 0){
    led1.setColor(BLYNK_LIGHT_YELLOW);
  } else if(a == 1){
    led1.setColor(BLYNK_YELLOW);
  } else if(a == 2){
    led1.setColor(BLYNK_ORANGE);
  } else if(a == 3){
    led1.setColor(BLYNK_RED);
  }
  
}






BLYNK_WRITE(V4){
  button = param.asInt();
}







void setup() {
  Serial.begin(9600);
  pinMode(ionizer, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  //timer.setInterval(1000L, read_sensor);
  
  led1.on();
  dht.begin();
  ds.begin();
  lcd.init();
  
  lcd.backlight();
  lcd.createChar(1, thermometro);
  lcd.createChar(2, igrasia);
  lcd.createChar(3, heart);
  lcd.createChar(4, clock1);
  lcd.clear();
  lcd.setCursor(0,0);

  Blynk.virtualWrite(V4, HIGH);

}






void loop() {
  Blynk.run();
  timer.run();
  ntp.update();
  ds.requestTemperatures();

  if((millis()-timer0) >= 1000){
    get_sensor();
    if(h <= 100){
      lcd_disp();
      Blynk.virtualWrite(V0, t);
      Blynk.virtualWrite(V1, h);
    }
    timer0 = millis();
  }

  if(button == 1){
    if(ntp.getMinutes() >= 30){
      digitalWrite(ionizer,HIGH);
    } else{
      digitalWrite(ionizer, LOW);
    }
  } else{
    digitalWrite(ionizer, LOW);
  }

}





void get_sensor(){
  h = dht.readHumidity();
  t = ds.getTempCByIndex(0);
  //if (isnan(h0) {
    //Serial.println("Failed to read from DHT sensor!");
    //return;
  //}

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C ");
  Serial.println();
}







void lcd_disp(){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("_____");
  lcd.setCursor(5, 0);
  lcd.write(4);
  if(ntp.getHours() < 10){
    lcd.print("0");
    lcd.print(ntp.getHours());
  } else{
    lcd.print(ntp.getHours());
  }
  lcd.setCursor(8, 0);
  lcd.print(":");
  if(ntp.getMinutes() < 10){
    lcd.print("0");
    lcd.print(ntp.getMinutes());
  } else{
    lcd.print(ntp.getMinutes());
  }
  lcd.setCursor(11, 0);
  lcd.print("_____");
  
  lcd.setCursor(1, 1);
  lcd.write(1);
  lcd.print(t);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(11, 1);
  lcd.write(2);
  lcd.print(h);
  lcd.print("%");

}
