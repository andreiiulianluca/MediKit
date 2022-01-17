#include <SoftwareSerial.h>
#include <PulseSensorPlayground.h>
#include <LiquidCrystal.h>
#include "DHT.h"
#define DHTTYPE DHT11
#define DHTPIN 13 //TH sensor

int redPin = 12;
int greenPin = 11;
int bTemperature, bHumidity = 0;
int threshold = 513; // Determine which signal to "count as a beat" and which to ignore.
int bpm = 0;
int calculatedBPM;
int Signal;
String mood; 
float t,h = 0;
const int pulsePin = 0;// PulseSensor connected to ANALOG PIN 0      
const int rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 9, d7 = 8;
unsigned long myTime;
unsigned long readTH;
unsigned long THOccurance;
unsigned long lastOccurance;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //Pins of the LED 16X2 Screen
SoftwareSerial sim(3, 2);
DHT dht(DHTPIN, DHTTYPE);

//Left for debugging sim module
void updateSerial()
{
    while (Serial.available()){
      sim.write(Serial.read());
    }
    while(sim.available()){
      Serial.write(sim.read());
    }
}

void sendSMS(int mode){
    sim.println("AT+CMGF=1");
    sim.println("AT+CMGS=\"+40747295501\"");
    switch(mode){
        case 0:
          sim.println("Check-Up Message");
          break;
        case 1:
          sim.println("DANGER!");
          sim.println("Your patient is in danger or the pulse sensor is not working properly");
          break;
        case 2:
          sim.println("DANGER!");
          sim.println("The temperatue it's too extreme!");
          break;
        case 3: 
          sim.println("DANGER!");
          sim.println("The humidity it's not in normal parameters!");
          break;
    }
    sim.print("Temperature: ");
    sim.println(t);
    sim.print("Humidity: ");
    sim.println(h);
    sim.print("BPM: ");
    sim.println(calculatedBPM);
    sim.print("Overall mood: ");
    sim.println(mood);
    sim.println((char)26);
}

void checkUp(){
    if (calculatedBPM!=0 && (calculatedBPM < 40 || calculatedBPM > 150)){
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, LOW);
        mood = "Red";
        sendSMS(1);
    }
    if ((calculatedBPM >= 40 && calculatedBPM <= 150) || calculatedBPM == 0){     
        digitalWrite(greenPin, HIGH);
        digitalWrite(redPin, LOW);
        mood = "Green";
        sendSMS(0);
    }  
    if (t < 16 || t > 25){
        mood = "Red";
        sendSMS(2);
    }
    if(h > 50 || h < 30){
        mood = "Red";
        sendSMS(3);
    }
}

void showTH(){
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.setCursor(2, 0);
    lcd.print(t);
    lcd.setCursor(8, 0);
    lcd.print("H:");
    lcd.setCursor(10, 0);
    lcd.print(h);
//  ALTERNATIVE (but blocking) WAY TO DISPLAY T&H
//  for (int positionCounter = 0; positionCounter < 32; positionCounter++) {
//      lcd.scrollDisplayLeft();
//      delay(400);
//  }
//  for (int positionCounter = 0; positionCounter < 48; positionCounter++) {
//      lcd.scrollDisplayRight();
//      delay(400);
//  }
//  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
//      lcd.scrollDisplayLeft();
//      delay(200);
//  }
}

void showBPM(){
    lcd.setCursor(0, 1);
    lcd.print("BPM: ");
    lcd.setCursor(5, 1);
    lcd.print(calculatedBPM);
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("DHT11 test!"));
    sim.begin(115200);
    lcd.begin(16, 2);
    dht.begin();
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void loop() {
    myTime = millis() - lastOccurance;
    readTH = millis() - THOccurance;
    Signal = analogRead(pulsePin);
    h = dht.readHumidity();        // read humidity            
    t = dht.readTemperature();     // read temperature
    if(Signal > threshold){
       bpm++;
    }
    if(myTime > 30000){
       calculatedBPM = bpm/10*2;
       showBPM();
       lastOccurance += 30000;
       bpm = 0;
    }
    //Good for testing sensor in plotter
    Serial.println(Signal);
    if(readTH > 65000){
      checkUp(); 
      THOccurance +=65000;
      showTH(); 
    }
    delay(10);
}
