/*
HIKARI PROJECT
open source base on arduino
by WOLFNEST-Studio
my IG
  @ redwolf_studio
  @ kitsune_film
--------------------------------------------------
Micro controller
 - atmega32a | atmega328p
Sensor
 - BH1750FVI (lux senser)
Displsy
 - 128x32 SSD1206 OLED display
--------------------------------------------------
*/
#include <EEPROM.h>

//Communication bus
#include <Wire.h>
#include <SPI.h>

//Lux sensor Lib
#include <BH1750FVI.h>      //by PeterEmbedded
//SSD1306 OLED Display Lib
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     32
#define OLED_RESET        -1

//light sennsor
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
//OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//pin
#define measureButt           2
#define SettingButt           3
#define ApeButt               4
#define ISOButt               5
#define PowerControl          7
#define batteryInd            0     // - analog A0

#define CALIBRATION_CONSTANT  250

float TIME;
String Shutter;
float SHUTTER_S;

float APERTURE;
float APERTURE_Spit;
double aperture_S;

uint16_t ISO;
float EV;
float lux;

uint8_t shutterSelect;
uint8_t AepSelect;
uint8_t isoSelect;

uint8_t page = 0;
boolean MODE = true;
boolean settingDisplay = false;
boolean autoShutdown;
uint32_t TimeShutdown;


//TEST
float FullBatteryVolt = 5.0;      //Full battery
float DisChargeBattery = 3.0;     //Low battery
/*
//9V Battery
float FullBatteryVolt = 9.0;      //Full battery
float DisChargeBattery = 5.0;     //Low battery
*/
/*
//Li-Po & Li-Ion @ 1cell
float FullBatteryVolt = 3.7;      //Full battery
float DisChargeBattery = 3.1;     //Low battery
*/
float batteryVolt;
float AnalogReadPin;
float R1 = 10000; //10K resister
float R2 = 10000; //10K resister
 


float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

//KiTSUNE LOGO
const unsigned char KiTSUNE [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x30, 0x40, 0x02, 0xe0, 0x80, 0x03, 0x81, 0x00, 0x02, 
  0x03, 0x00, 0x06, 0x02, 0x00, 0x04, 0x02, 0x00, 0x0c, 0x02, 0x80, 0x0c, 0x03, 0x80, 0x08, 0x03, 
  0x00, 0x08, 0x01, 0x80, 0x18, 0x01, 0x80, 0x18, 0x00, 0xc0, 0x1c, 0x00, 0xc0, 0x0d, 0x04, 0xc0, 
  0x0d, 0x44, 0xc0, 0x07, 0x66, 0xc0, 0x07, 0x27, 0xc0, 0x03, 0xb3, 0xc0, 0x03, 0xff, 0x80, 0x03, 
  0xff, 0x80, 0x03, 0xff, 0x80, 0x07, 0xff, 0x80, 0x07, 0xff, 0x80, 0x0f, 0xff, 0x80, 0x0f, 0xff, 
  0x00, 0x3f, 0xff, 0x00, 0x7f, 0xfe, 0x00, 0x3f, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00
};
/*
******************************************************************************************
*                                                                                        *
*                                 Void MAINDISPLAY_Amode                                 *
*                                                                                        *
******************************************************************************************
*/
void MAINDISPLAY_Amode() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.println(F("TIME"));
  display.setTextSize(2);

  //Show Shutter speed
  if(TIME >= 11500) {
    display.print(F("OVER"));
  }
  if(TIME >= 1.5 && TIME < 11500) {
    display.print(F("1/"));
    display.print(Shutter);
  }
  if(TIME < 1.5) {
    display.print(1 / TIME);
  }

  display.setTextSize(1);

  //Show aperture
  display.setCursor(75, 1);
  display.print(F("f/"));
  display.setCursor(95, 1);
  display.println(APERTURE, 2);

  //Show ISO
  display.setCursor(75, 10);
  display.print(F("ISO"));
  display.setCursor(95, 10);
  display.println(ISO);

  //Show EV
  display.setCursor(75, 20);
  display.print(F("EV"));
  display.setCursor(95, 20);
  display.println(EV);

  //Battery indicator
  display.fillRect(3, 25, 11, 6, SSD1306_WHITE);
  display.fillRect(1, 26, 3, 4, SSD1306_WHITE);
  display.fillRect(4, 26, mapFloat(batteryVolt, DisChargeBattery, FullBatteryVolt, 9, 0), 4, SSD1306_BLACK);
  


  display.display();
}//void MAINDISPLAY_Amode
/*
******************************************************************************************
*                                                                                        *
*                                 Void MAINDISPLAY_Smode                                 *
*                                                                                        *
******************************************************************************************
*/
void MAINDISPLAY_Smode()  {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.println(F("APERTURE"));
  display.setTextSize(2);

  //Show APERTURE
  if(APERTURE_Spit == 0) {
    display.print(F("LIMIT"));
  }
  if(APERTURE_Spit >= 23) {
    display.print(F("OVER"));
  }
  if(APERTURE_Spit > 0 && APERTURE_Spit < 23) {
    display.print(APERTURE_Spit);
  }

  display.setTextSize(1);

  //Show speedShutter
  display.setCursor(65, 1);
  display.print(F("S"));
  display.setCursor(85, 1);
  switch(shutterSelect) {
        case 0 : display.print(F("30"));
                  break;
        case 1 : display.print(F("15"));
                  break;
        case 2 : display.print(F("8"));
                  break;
        case 3 : display.print(F("4"));
                  break;
        case 4 : display.print(F("2"));
                  break;
        case 5 : display.print(F("1"));
                  break;
        case 6 : display.print(F("1/2"));
                  break;
        case 7 : display.print(F("1/8"));
                  break;
        case 8 : display.print(F("1/15"));
                  break;
        case 9 : display.print(F("1/30"));
                  break;
        case 10 : display.print(F("1/60"));
                  break;
        case 11 : display.print(F("1/125"));
                  break;
        case 12 : display.print(F("1/250"));
                  break;
        case 13 : display.print(F("1/500"));
                  break;
        case 14 : display.print(F("1/1000"));
                  break;
        case 15 : display.print(F("1/2000"));
                  break;
        case 16 : display.print(F("1/4000"));
                  break;
        case 17 : display.print(F("1/8000"));
                  break;
      }
  


  //Show ISO
  display.setCursor(65, 10);
  display.print(F("ISO"));
  display.setCursor(85, 10);
  display.println(ISO);

  //Show EV
  display.setCursor(65, 20);
  display.print(F("EV"));
  display.setCursor(85, 20);
  display.println(EV);

  //Battery indicator
  display.fillRect(3, 25, 11, 6, SSD1306_WHITE);
  display.fillRect(1, 26, 3, 4, SSD1306_WHITE);
  display.fillRect(4, 26, mapFloat(batteryVolt, DisChargeBattery, FullBatteryVolt, 9, 0), 4, SSD1306_BLACK);
  


  display.display();
}


/*
******************************************************************************************
*                                                                                        *
*                                      Void SETTING                                      *
*                                                                                        *
******************************************************************************************
*/
void SETTING() {
  //Control
  if(digitalRead(ApeButt) == LOW && digitalRead(measureButt) == HIGH && settingDisplay == true) { //Next page
    page++;
    delay(100);
  }
  if(digitalRead(ISOButt) == LOW && digitalRead(measureButt) == HIGH && settingDisplay == true) { //Befor page
    page--;
    delay(100);
  }
  if(page == 255) {
    page = 0;
  }
  if(page == 5) {
    page = 0;
  }


  //Display
  switch (page) {
    case 0:
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.println(F("MODE"));
    display.setCursor(1,15);
    display.setTextSize(2);
    
    if(digitalRead(measureButt) == LOW && digitalRead(ApeButt) == LOW) {
      MODE = true;
    }
    if(digitalRead(measureButt) == LOW && digitalRead(ISOButt) == LOW) {
      MODE = false;
    }

    if(MODE == true) {
      display.print("S");
    }
    if(MODE == false) {
      display.print("A");
    }
    display.display();
    break;

  case 1:
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.println(F("AUTO SHUTDOWN"));
    display.setCursor(1,15);
    display.setTextSize(2);
    
    if(digitalRead(measureButt) == LOW && digitalRead(ApeButt) == LOW) {
      autoShutdown = false;
    }
    if(digitalRead(measureButt) == LOW && digitalRead(ISOButt) == LOW) {
      autoShutdown = true;
    }

    if(autoShutdown == true) {
      display.print("ON");
    }
    if(autoShutdown == false) {
      display.print("OFF");
    }
    display.display();
    break;
  
  case 2:
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.println(F("TIME SHUTDOWN (min)"));
    display.setCursor(1,15);
    display.setTextSize(2);
    display.print(TimeShutdown/60000.00, 2);

    if(digitalRead(measureButt) == LOW && digitalRead(ApeButt) == LOW) {
      TimeShutdown = TimeShutdown + 15000;
      delay(100);
    }
    if(digitalRead(measureButt) == LOW && digitalRead(ISOButt) == LOW) {
      TimeShutdown = TimeShutdown - 15000;
      delay(100);
    }
    if(TimeShutdown < 30000) { //min 30sce
      TimeShutdown = 30000;
    }
    if(TimeShutdown > 300000) { //max 5min
      TimeShutdown = 300000;
    }

    display.display();
    break;

  case 3:
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.println(F("BATTERY INFO"));
    display.setCursor(1,15);
    display.setTextSize(2);
    display.print(mapFloat(batteryVolt, DisChargeBattery, FullBatteryVolt, 0, 100), 1);
    display.print(" %");
    display.display();
    break;

  case 4:
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.println(F("FACTORY RESET"));

    boolean EnterEvent = false;
    if(digitalRead(measureButt) == LOW) {
      EnterEvent = true;
    }
    while (EnterEvent == true) {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,1);
      display.print(F("Are you sure?"));
      display.setCursor(1,15);
      display.println(F("F1 = Yes"));
      display.print(F("F2 = No"));
      display.display();

      if(digitalRead(ApeButt) == LOW) {
        //Format EEPROM
        for(uint16_t address=0; address<EEPROM.length(); address++) {
          EEPROM.update(address, 0);

          display.clearDisplay();
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(1,1);
          display.setTextSize(1);
          display.print(F("CLEAR ADDRESS"));
          display.setCursor(1,15);
          display.setTextSize(2);
          display.print(address);
          display.display();

       }
        //Reset value
        autoShutdown = true;
        TimeShutdown = 60000;
        EnterEvent = false;
        MODE = false;
      }
      if(digitalRead(ISOButt) == LOW) {
        EnterEvent = false;
      }
    }
    break;
  }//Switch

  display.display();

}//void SETTING


/*
******************************************************************************************
*                                                                                        *
*                                       ERROR REPORT                                     *
*                                                                                        *
******************************************************************************************
*/
void showErr(String ErrorCode, String Message) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(1,1);
  display.print(ErrorCode);
  display.setCursor(1,10);
  display.print(Message);
  display.display();
}

/*
******************************************************************************************
*                                                                                        *
*                                    i2c EEPROM Driver                                   *
*                                                                                        *
******************************************************************************************
*/
/*
//Write EEPROM
void writeAddress(byte i2cAddress, int address, byte val) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.write(val);
  Wire.endTransmission();
  delay(5);
}
//Read EEPROM
byte readAddress(byte i2cAddress, int address) {
  byte rData = 0xFF;
  
  Wire.beginTransmission(i2cAddress);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.endTransmission();  
  Wire.requestFrom(i2cAddress, 1);
  rData =  Wire.read();
  return rData;
}
*/

/*
******************************************************************************************
*                                                                                        *
*                                      Void Setup                                        *
*                                                                                        *
******************************************************************************************
*/
void setup() {
  Wire.begin();
  Serial.begin(9600);
  LightSensor.begin();

  pinMode(measureButt, INPUT_PULLUP);
  pinMode(SettingButt, INPUT_PULLUP);
  pinMode(ApeButt, INPUT_PULLUP);
  pinMode(ISOButt, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  //Report Error
  while(!Wire.requestFrom(0x23, 1)) {
    showErr("Err01", "No Sensor");
  }


  //power management
  digitalWrite(PowerControl, HIGH);
  AnalogReadPin = (analogRead(batteryInd) * 5.0) / 1024.0;
  batteryVolt = AnalogReadPin / (R2/(R1+R2));
  while(batteryVolt <= DisChargeBattery) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.print(F("LOW BATTERY"));
    display.display();
    delay(1000);
    digitalWrite(PowerControl, LOW);
  }

  //READ EEPROM
  autoShutdown  =   EEPROM.get(0, autoShutdown);
  TimeShutdown  =   EEPROM.get(1, TimeShutdown);
  AepSelect     =   EEPROM.get(5, AepSelect);
  APERTURE      =   EEPROM.get(6, APERTURE);
  isoSelect     =   EEPROM.get(10, isoSelect);
  ISO           =   EEPROM.get(11, ISO);
  shutterSelect =   EEPROM.get(13, shutterSelect);
  SHUTTER_S     =   EEPROM.get(14, SHUTTER_S);
  MODE          =   EEPROM.get(18, MODE);


  //Show Logo
  display.clearDisplay();
  display.display();
  delay(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,10);
  display.setTextSize(2);
  display.println(F("KiTSUNE"));
  display.drawBitmap(95,0, KiTSUNE, 20, 32, SSD1306_WHITE);
  display.display();
  delay(2000);
  display.fillScreen(SSD1306_BLACK);
  display.display();


  //Serial Debug
  Serial.println("********************************* Serial Debug *********************************");
  Serial.println("-- READ VALUE STARTUP --");
  Serial.print("AutoSleepEvent : "); Serial.println(autoShutdown);
  Serial.print("TimeSleepEvent : "); Serial.println(TimeShutdown);
  Serial.print("AepSelect : "); Serial.println(AepSelect);
  Serial.print("A : "); Serial.println(APERTURE);
  Serial.print("ISOSelect : "); Serial.println(isoSelect);
  Serial.print("ISO : "); Serial.println(ISO);
  Serial.print("shutterSelect : "); Serial.println(shutterSelect);
  Serial.print("SHUTTER_S : "); Serial.println(SHUTTER_S);
  Serial.print("MODE : "); Serial.println(MODE);
  Serial.println("-- SENSOR READ --");
  Serial.print("AnalogReadPin : "); Serial.println(AnalogReadPin);
  Serial.print("batteryVolt : "); Serial.println(batteryVolt);
  Serial.println("********************************************************************************");
  delay(200);


}//void setup


/*
******************************************************************************************
*                                                                                        *
*                                       Void loop                                        *
*                                                                                        *
******************************************************************************************
*/
void loop() {

  /*
  voltage divider 
  max voltage           = 10Valt 
  Vref                  = 5V 
  resolution of ADC     = 10bit
  */
  AnalogReadPin = (analogRead(batteryInd) * 5.0) / 1024.0;
  batteryVolt = AnalogReadPin / (R2/(R1+R2));
  
  //Get lux
  lux = LightSensor.GetLightIntensity() * 2.17;
  //Get EV
  EV = (log10(lux * ISO / CALIBRATION_CONSTANT) / log10(2));

  //mode A
  if(MODE == false) {
    //Get Speed shutter (mod A)
    TIME = pow(2, EV) / pow(APERTURE, 2);

    //Shutter spriter
    if(TIME >= 1.5) {
      Shutter = 2;
    }
    if(TIME >= 3) {
      Shutter = 4;
    }
    if(TIME >= 6) {
      Shutter = 8;
    }
    if(TIME >= 11.5) {
      Shutter = 15;
    }
    if(TIME >= 22.5) {
      Shutter = 30;
    }
    if(TIME >= 45) {
      Shutter = 60;
    }
    if(TIME >= 92.5) {
      Shutter = 125;
    }
    if(TIME >= 187.5) {
      Shutter = 250;
    }
    if(TIME >= 375) {
      Shutter = 500;
    }
    if(TIME >= 750) {
      Shutter = 1000;
    }
    if(TIME >= 1500) {
      Shutter = 2000;
    }
    if(TIME >= 3000) {
      Shutter = 4000;
    }
    if(TIME >= 6000) {
      Shutter = 8000;
    }
    

    //APERTURE
    if(digitalRead(ApeButt) == LOW && settingDisplay == false) {
      AepSelect++;
      if(AepSelect >= 29) {
        AepSelect=0;
      }
      switch(AepSelect) {
        case 0 : APERTURE = 0.95;
                  break;
        case 1 : APERTURE = 1.0;
                  break;
        case 2 : APERTURE = 1.1;
                  break;
        case 3 : APERTURE = 1.2;
                  break;
        case 4 : APERTURE = 1.4;
                  break;
        case 5 : APERTURE = 1.6;
                  break;
        case 6 : APERTURE = 1.8;
                  break;
        case 7 : APERTURE = 2.0;
                  break;
        case 8 : APERTURE = 2.2;
                  break;
        case 9 : APERTURE = 2.5;
                  break;
        case 10 : APERTURE = 2.8;
                  break;
        case 11 : APERTURE = 3.2;
                  break;
        case 12 : APERTURE = 3.5;
                  break;
        case 13 : APERTURE = 4.0;
                  break;
        case 14 : APERTURE = 4.5;
                  break;
        case 15 : APERTURE = 5.0;
                  break;
        case 16 : APERTURE = 5.6;
                  break;
        case 17 : APERTURE = 6.3;
                  break;
        case 18 : APERTURE = 7.1;
                  break;
        case 19 : APERTURE = 8.0;
                  break;
        case 20 : APERTURE = 9.0;
                  break;
        case 21 : APERTURE = 10;
                  break;
        case 22 : APERTURE = 11;
                  break;
        case 23 : APERTURE = 13;
                  break;
        case 24 : APERTURE = 14;
                  break;          
        case 25 : APERTURE = 16;
                  break;
        case 26 : APERTURE = 18;
                  break;
        case 27 : APERTURE = 20;
                  break;
        case 28 : APERTURE = 22;
                  break;
      }
      delay(500);
       MAINDISPLAY_Amode();
    }
  }

  //mode S
  if(MODE == true) {
    //Get Aperture (mod S)
    if(shutterSelect >= 0) {
      aperture_S = sqrt((lux * ISO * SHUTTER_S) / CALIBRATION_CONSTANT);
    }
    if(shutterSelect > 5) {
      aperture_S = sqrt((lux * ISO * (1/SHUTTER_S)) / CALIBRATION_CONSTANT);
    }

    //APERTURE spiter
    if(aperture_S < 0.95) {
      APERTURE_Spit = 0;
    }
    if(aperture_S >= 0.95) {
      APERTURE_Spit = 0.95;
      AepSelect = 0;
    }
    if(aperture_S >= 0.97) {
      APERTURE_Spit = 1.0;
      AepSelect = 1;
    }
    if(aperture_S >= 1.05) {
      APERTURE_Spit = 1.1;
      AepSelect = 2;
    }
    if(aperture_S >= 1.15) {
      APERTURE_Spit = 1.2;
      AepSelect = 3;
    }
    if(aperture_S >= 1.3) {
      APERTURE_Spit = 1.4;
      AepSelect = 4;
    }
    if(aperture_S >= 1.5) {
      APERTURE_Spit = 1.6;
      AepSelect = 5;
    }
    if(aperture_S >= 1.7) {
      APERTURE_Spit = 1.8;
      AepSelect = 6;
    }
    if(aperture_S >= 1.9) {
      APERTURE_Spit = 2.0;
      AepSelect = 7;
    }
    if(aperture_S >= 2.1) {
      APERTURE_Spit = 2.2;
      AepSelect = 8;
    }
    if(aperture_S >= 2.35) {
      APERTURE_Spit = 2.5;
      AepSelect = 9;
    }
    if(aperture_S >= 2.65) {
      APERTURE_Spit = 2.8;
      AepSelect = 10;
    }
    if(aperture_S >= 3.0) {
      APERTURE_Spit = 3.2;
      AepSelect = 11;
    }
    if(aperture_S >= 3.35) {
      APERTURE_Spit = 3.5;
      AepSelect = 12;
    }
    if(aperture_S >= 3.75) {
      APERTURE_Spit = 4.0;
      AepSelect = 13;
    }
    if(aperture_S >= 4.25) {
      APERTURE_Spit = 4.5;
      AepSelect = 14;
    }
    if(aperture_S >= 4.75) {
      APERTURE_Spit = 5.0;
      AepSelect = 15;
    }
    if(aperture_S >= 5.3) {
      APERTURE_Spit = 5.6;
      AepSelect = 16;
    }
    if(aperture_S >= 5.95) {
      APERTURE_Spit = 6.3;
      AepSelect = 17;
    }
    if(aperture_S >= 6.7) {
      APERTURE_Spit = 7.1;
      AepSelect = 18;
    }
    if(aperture_S >= 7.55) {
      APERTURE_Spit = 8.0;
      AepSelect = 19;
    }
    if(aperture_S >= 8.5) {
      APERTURE_Spit = 9.0;
      AepSelect = 20;
    }
    if(aperture_S >= 9.5) {
      APERTURE_Spit = 10.0;
      AepSelect = 21;
    }
    if(aperture_S >= 10.5) {
      APERTURE_Spit = 11.0;
      AepSelect = 22;
    }
    if(aperture_S >= 12.00) {
      APERTURE_Spit = 13.0;
      AepSelect = 23;
    }
    if(aperture_S >= 13.5) {
      APERTURE_Spit = 14.0;
      AepSelect = 24;
    }
    if(aperture_S >= 15.00) {
      APERTURE_Spit = 16.0;
      AepSelect = 25;
    }
    if(aperture_S >= 17.00) {
      APERTURE_Spit = 18.0;
      AepSelect = 26;
    }
    if(aperture_S >= 19.00) {
      APERTURE_Spit = 20.0;
      AepSelect = 27;
    }
    if(aperture_S >= 21.00) {
      APERTURE_Spit = 22.0;
      AepSelect = 28;
    }
    if(aperture_S >= 23.50) {
      APERTURE_Spit = 23;
    }

    
    //SHUTTER SPEED Selecter
    if(digitalRead(ApeButt) == LOW && settingDisplay == false) {
      shutterSelect++;
      if(shutterSelect >= 18) {
        shutterSelect=0;
      }
      switch(shutterSelect) {
        case 0 : SHUTTER_S = 30;
                  break;
        case 1 : SHUTTER_S = 15;
                  break;
        case 2 : SHUTTER_S = 8;
                  break;
        case 3 : SHUTTER_S = 4;
                  break;
        case 4 : SHUTTER_S = 2;
                  break;
        case 5 : SHUTTER_S = 1;
                  break;
        case 6 : SHUTTER_S = 2;
                  break;
        case 7 : SHUTTER_S = 8;
                  break;
        case 8 : SHUTTER_S = 15;
                  break;
        case 9 : SHUTTER_S = 30;
                  break;
        case 10 : SHUTTER_S = 60;
                  break;
        case 11 : SHUTTER_S = 125;
                  break;
        case 12 : SHUTTER_S = 250;
                  break;
        case 13 : SHUTTER_S = 500;
                  break;
        case 14 : SHUTTER_S = 1000;
                  break;
        case 15 : SHUTTER_S = 2000;
                  break;
        case 16 : SHUTTER_S = 4000;
                  break;
        case 17 : SHUTTER_S = 8000;
                  break;
      }
      delay(500);
       MAINDISPLAY_Smode();
    }
  }




  //ISO
  if(digitalRead(ISOButt) == LOW && settingDisplay == false) {
    isoSelect++;
    if(isoSelect >= 28) {
      isoSelect=0;
    }
    switch (isoSelect) {
      case 0 : ISO = 12;
                break;
      case 1 : ISO = 16;
                break;
      case 2 : ISO = 20;
                break;
      case 3 : ISO = 25;
                break;
      case 4 : ISO = 32;
                break;
      case 5 : ISO = 40;
                break;
      case 6 : ISO = 50;
                break;
      case 7 : ISO = 64;
                break;
      case 8 : ISO = 80;
                break;
      case 9 : ISO = 100;
                break;
      case 10 : ISO = 125;
                break;
      case 11 : ISO = 160;
                break;
      case 12 : ISO = 200;
                break;
      case 13 : ISO = 250;
                break;
      case 14 : ISO = 320;
                break;
      case 15 : ISO = 400;
                break;
      case 16 : ISO = 500;
                break;
      case 17 : ISO = 640;
                break;
      case 18 : ISO = 800;
                break;
      case 19 : ISO = 1000;
                break;
      case 20 : ISO = 1250;
                break;
      case 21 : ISO = 1600;
                break;
      case 22 : ISO = 2000;
                break;
      case 23 : ISO = 2500;
                break;
      case 24 : ISO = 3200;
                break;
      case 25 : ISO = 4000;
                break;
      case 26 : ISO = 5000;
                break;
      case 27 : ISO = 6400;
                break;
    }
    delay(500);
    if(MODE == false) {
      MAINDISPLAY_Amode();
    }
    if(MODE == true) {
      MAINDISPLAY_Smode();
    }
  }



  //Measure
  if(digitalRead(measureButt) == LOW && settingDisplay == false) {
    if(MODE == false) {
      MAINDISPLAY_Amode();
    }
    if(MODE == true) {
      MAINDISPLAY_Smode();
    }

    //Save EEPROM
    EEPROM.put(5, AepSelect);
    EEPROM.put(6, APERTURE);
    EEPROM.put(10, isoSelect);
    EEPROM.put(11, ISO);
    EEPROM.put(13, shutterSelect);
    EEPROM.put(14, SHUTTER_S);
    EEPROM.put(18, MODE);

    delay(100);
  }

  //Setting
  if(digitalRead(SettingButt) == LOW && settingDisplay == false) {
    settingDisplay = true;
    delay(100);
  }
  while(settingDisplay == true) {
    SETTING();
    if(digitalRead(SettingButt) == LOW) {
      settingDisplay = false;
      display.fillScreen(SSD1306_BLACK);
      display.display();

      //Save EEPROM
      EEPROM.put(0, autoShutdown);
      EEPROM.put(1, TimeShutdown);
      delay(100);
    }
  }
  
  //Save display
  if(autoShutdown == true) {
    static unsigned long Shutdown = millis();
    if(digitalRead(measureButt) == HIGH && digitalRead(ApeButt) == HIGH && digitalRead(ISOButt) == HIGH && settingDisplay == false) {
      if((millis() - Shutdown) > TimeShutdown) {
        //shuttdown
        display.fillScreen(SSD1306_BLACK);    //Remove soon
        display.display();                    //Remove soon
        //EEPROM SAVE
        EEPROM.put(0, autoShutdown);
        EEPROM.put(1, TimeShutdown);
        EEPROM.put(5, AepSelect);
        EEPROM.put(6, APERTURE);
        EEPROM.put(10, isoSelect);
        EEPROM.put(11, ISO);
        EEPROM.put(13, shutterSelect);
        EEPROM.put(14, SHUTTER_S);
        EEPROM.put(18, MODE);
        delay(100);
        digitalWrite(PowerControl, LOW); //Shutdow system
        
        Shutdown = millis();
      }
    }
    if(digitalRead(measureButt) == LOW || digitalRead(ApeButt) == LOW || digitalRead(ISOButt) == LOW || settingDisplay == true) {
      Shutdown = millis();
    }

  }

  //LOW BATTERY
  while(batteryVolt <= DisChargeBattery) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,1);
    display.setTextSize(1);
    display.print(F("LOW BATTERY"));
    display.display();
    delay(1000);
    digitalWrite(PowerControl, LOW);
  }
}//void loop
