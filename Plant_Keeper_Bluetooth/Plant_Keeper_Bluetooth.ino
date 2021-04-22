#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

SoftwareSerial hcSerial(3, 2);  // RX, TX

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //I2C pins declaration

#define mixpump 9       //pump for mix solution
#define pump 10         //pump for the plant

#define moistpin A0     //Capacitive Soil Moisture Sensor

#define echopin 5      //Sensor for water tank level
#define trigpin 6

#define mintank 50      //lowest level  - 50cm tank water level from sensor
#define maxtank 10      //highest level - 10cm tank water level from sensor
#define tankLOW 40      //40cm from sensor is the minimum water level accept

#define DHTPIN 7        //pin D7 sensor for temperatur and humidity
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

#define moistmin 900    //soil moisture minimum treshold
#define moistmax 650    //soil moisture maximum treshold
#define moistlevel 750  //moisture level for soil to be watered

//minimum 1023
//maximum 650
//watering level 750

char input;
unsigned long changeTime;
unsigned long changeTime2;
unsigned long timeStable;

byte currState, lastState;

bool checkState;

int temp;
int hum;
int moistsensor, moist;
int duration, distance, tank, tankstatus;

void setup() {
  Serial.begin(9600);
  hcSerial.begin(9600);

  delay(3000);

  pinMode(moistpin, INPUT);
  pinMode(echopin, INPUT);
  pinMode(DHTPIN, INPUT);
  
  pinMode(trigpin, OUTPUT);
  pinMode(mixpump, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(11, OUTPUT);

  dht.begin();

  lcd.begin(16,2);//Defining 16 columns and 2 rows of lcd display
  lcd.backlight();//To Power ON the back light
  //lcd.noBacklight();// To Power OFF the back light


  //Start System
  mixpumpOFF();
  pumpOFF();
  
  //INTRO Display
  lcd.setCursor(0,0); lcd.print("  Welcome to");
  lcd.setCursor(0,1); lcd.print(" Plant Keeper");
  Serial.println();
  Serial.println("Welcome to Plant Keeper");

  melody();


  //Sensor Check
  sensorCheck();    //check sensor status

  displayStatus();
  delay(1000);

  while (tank < 30) {
    tankempty();
    sensorCheck();
  }

/*
  //Run the mixture pump to ready
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Mix Pump ON");
  lcd.setCursor(0,1); lcd.print("for a moment...");
  Serial.println("System is preparing.....");

  
  //Ready the system
  pumpOFF();
  mixpumpON();
  delay(10000);     //ON mixPump for 10 seconds
  mixpumpOFF();
*/

  lcd.clear();
  lcd.setCursor(0,1); lcd.print("System ready");
  Serial.println("System will be ready in 3 seconds...");
  delay(500);
  lcd.setCursor(0,1); lcd.print("System ready.");
  delay(500);
  lcd.setCursor(0,1); lcd.print("System ready..");
  delay(500);
  lcd.setCursor(0,1); lcd.print("System ready...");
  delay(500);
  lcd.setCursor(0,1); lcd.print("System ready....");
  delay(500);

  automatic();

  lastState = HIGH;
  checkState = false;
  
}

void loop() {

  sensorCheck();    //check sensor status

  displayStatus();
  delay(100);

  if (tank < 30) {
    tankempty();
  }

  else {
    
  //begin of pump
  
    if(hcSerial.available()) {
  
      input = hcSerial.read();
  
      Serial.print("   Input:");
      Serial.print(input);
  
      switch(input) {
        case 'a': automatic(); break;
        case 'm': manual(); break;
        case 'p': pumpOFF(); break;
        case 'P': pumpON(); break;
        case '1': changeTime = millis(); break;
      }

    }

    if ((millis() - changeTime) > 30000) {
      digitalWrite(11, HIGH);
      automatic();
    }
    else digitalWrite(11, LOW);
  
    //end of pump
  }

}


void automatic() {
  if (moistsensor > moistlevel) {

    if (moistsensor >= moistlevel) currState = LOW;
    if (moistsensor < moistlevel) currState = HIGH;
  
    if (currState != lastState) {
      if (currState == LOW) {
        timeStable = millis();
        checkState = true;
      }
    }
  
      else {
        checkState = true;
      }
  
      lastState = currState;
  
    if (checkState) {
      if ((millis() - timeStable) >= 180000) {        //stable LOW state for 3 minits (180second)
        //once moist level LOW is stable state
        mixpumpON();
        if ((millis() - changeTime2) > 30000) {     //pumpON after 30 seconds mixpumpON
          pumpON();
        }   //end of LOW level stable state
      }

      else {
        changeTime2 = millis();
        //pumpOFF();
        //mixpumpOFF();
      }
    }

    

      
  }
  else {
    mixpumpOFF();
    pumpOFF();
    

    lastState = HIGH;
    checkState = false;
  }
}


void manual() {
  mixpumpON();
}


void mixpumpON() {
  digitalWrite(mixpump, HIGH);
}


void mixpumpOFF() {
  digitalWrite(mixpump, LOW);
}


void pumpON() {
  digitalWrite(pump, HIGH);
}


void pumpOFF() {
  digitalWrite(pump, LOW);
}


void tankempty() {
  mixpumpOFF();
  pumpOFF();
  
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(" TANK LEVEL LOW");
  lcd.setCursor(0,1); lcd.print("  SYSTEM STOP");

  Serial.println();
  Serial.println("Tank is EMPTY. Please REFILL");

  #define NOTE_A5   880
  #define NOTE_CS6   1109
  
  #define BUZZER 8
  
  int melody[] = {NOTE_CS6, NOTE_A5};
  int noteDurations[] = {4, 4};
  
    for (int thisNote = 0; thisNote < 2; thisNote++){
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER);
    }

  delay(500);
  lcd.clear();
}


void sensorCheck() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  moistsensor = analogRead(moistpin);

  moist = map(moistsensor,moistmin,moistmax,0,100);
  if (moist < 0) moist = 0;
  
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigpin, LOW);
  duration = pulseIn(echopin, HIGH);
  distance = (duration / 2) / 29.1;
  tank = map(distance,mintank,maxtank,0,100);
  if (tank < 0) tank = 0;
  if (distance < tankLOW) tankstatus = 100;
  if (distance > tankLOW) tankstatus = 30;
  
  if (isnan(hum) || isnan(temp)) {
    Serial.println("DHT error!");
    return;
  }

  hcSerial.print(temp);
  hcSerial.print("|");
  hcSerial.print(hum);
  hcSerial.print("|");
  hcSerial.print(moist);
  hcSerial.print("|");
  hcSerial.print(tank);
  hcSerial.print("|");
  hcSerial.print(tankstatus);
  hcSerial.println("|");

  Serial.println("");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C   Humidity: ");
  Serial.print(hum);
  Serial.print("   Soil Moisture: ");
  Serial.print(moist);
  Serial.print("%   Water Tank: ");
  Serial.print(tank);
  Serial.print("%");

}


void displayStatus() {
  lcd.setCursor(0,0); lcd.print("Temp:");
  lcd.setCursor(5,0); lcd.print(temp);
  lcd.setCursor(9,0); lcd.print("Hum:");
  lcd.setCursor(13,0); lcd.print(hum);
  lcd.setCursor(0,1); lcd.print("Soil:");
  lcd.setCursor(5,1); lcd.print(moist);
  if(moist >= 100){lcd.setCursor(8,1); lcd.print("Tank:");}
  if(moist < 100){lcd.setCursor(7,1); lcd.print(" Tank:");}
  if(moist < 10){lcd.setCursor(6,1); lcd.print("  Tank:");}
  lcd.setCursor(13,1); lcd.print(tank);
  if(tank < 100){lcd.setCursor(15,1); lcd.print(" ");}
  
}


void melody(){

#define NOTE_B5   988
#define NOTE_G5   784
#define NOTE_A5   880
#define NOTE_DS6  1245
#define NOTE_CS6   1109

#define BUZZER 8

int melody[] = {NOTE_B5, 0, NOTE_G5, NOTE_A5, NOTE_B5, 0, NOTE_G5, NOTE_A5, NOTE_B5, 0, NOTE_B5, NOTE_B5, NOTE_DS6, NOTE_DS6, NOTE_B5, NOTE_B5, NOTE_CS6};
int noteDurations[] = {4, 16, 8, 4, 4, 16, 8, 4, 4, 8, 4, 8, 4, 8, 4, 8, 2};

  for (int thisNote = 0; thisNote < 17; thisNote++){
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER);
  }
}
