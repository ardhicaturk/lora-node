#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  433E6

unsigned int counter = 0;

SSD1306 display(0x3c, 4, 15);
String rssi = "RSSI --";

typedef struct{
    float v1;
    float a1;
    float v2;
    float a2;
} kirim;
float wcsValue(int in){
    float out;
    out = 0.0269 * (float(in)) + 1.7323;
    return out;
}

float zmptValue(int in){
    float out;
    float a = float(in/4096.0) ;
    out = (a * 250 * 2) ;
    return out;
}
void setup() {
  pinMode(16,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Sender");
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  LoRa.receive();
  Serial.println("init ok");
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
   
}

unsigned long time1;
unsigned long time2;
void loop() {
    kirim k;
    k.v1 = zmptValue(analogRead(A4));
    k.a1 = wcsValue(analogRead(A5));
    k.v2 = zmptValue(analogRead(A6));
    k.a2 = wcsValue(analogRead(A7));
    
    if (millis() - time2 > 200){
        time2 = millis();
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);

        display.drawString(0, 0, "Client: ");
        display.drawString(50, 0, String(counter));
        display.drawString(0, 12, "Voltage1: ");
        display.drawString(50, 12, String(k.v1) + 'V');
        display.drawString(0, 24, "Current1: ");
        display.drawString(50, 24, String(k.a1) + 'A');
        display.drawString(0, 36, "Voltage2: ");
        display.drawString(50, 36, String(k.v2) + 'V');
        display.drawString(0, 48, "Current2: ");
        display.drawString(50, 48, String(k.a2) + 'A');
        display.display();
    }
    

    // send packet
    if(millis() - time1 > 1000){
        time1 = millis();
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&k , sizeof(k));
        LoRa.endPacket();
    }

}
