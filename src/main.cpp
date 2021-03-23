#include <Arduino.h>
/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int switch1_pin = 12;
const int switch2_pin = 13;
int switchStatus1_Last = LOW;  // last status switch
int switchStatus1;
int switchStatus2_Last = LOW;  // last status switch
int switchStatus2;
int sens_count;        
//------------------------------------------------------------------------
#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
float BPM, SpO2;
uint32_t tsLastReport = 0;
 
const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PulseOximeter pox;

///******************************************************************************************
// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
    display.drawBitmap( 100, 32, bitmap, 28, 28, 1);
    display.display();
}
///******************************************************************************************
void PulseOxi()
{
  String hr = String(pox.getHeartRate(),0);
  String spo = String(pox.getSpO2());
  display.clearDisplay();
  display.setTextSize(1); 
  display.setCursor(0,0);
  display.println("PulseOximeter");
  
  if ((pox.getSpO2())<20)
  {
    Serial.println("Finger Not Detect");

    display.setTextSize(2); 
    display.setCursor(0,20);
    display.println("Finger Not  Detect");
  }
  else
  {
    Serial.print("Heart rate:");
    Serial.print(hr);
    Serial.print("bpm / SpO2:");
    Serial.print(spo);
    Serial.println("%");
 
    display.setTextSize(2);  
    display.setCursor(0,20);
    display.println("HR: " + hr);
    display.setCursor(0,40);
    display.println("O2: " + spo);
  }
  
 display.display();

}
///******************************************************************************************
void setup()
{
  Serial.begin(9600);

  
  pinMode(switch1_pin, INPUT);
  pinMode(switch2_pin, INPUT);
// MAX30100 ----------------------------------------------------------------------------------    
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    
    Serial.print("Initializing pulse oximeter..");
    display.setTextSize(1);  
    display.setCursor(0,20);
    display.println("Initializing pulse oximeter..");
    if (!pox.begin()) 
      {
        Serial.println("FAILED");
        display.setTextSize(2);  
        display.setCursor(0,20);
        display.println("SPO2 FAILED");
        for(;;);
      } 
    else 
      {
        Serial.println("SUCCESS");
        display.setTextSize(2);  
        display.setCursor(0,20);
        display.println("SPO2 SUCCESS");
      }

    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

// SSD1306 --------------------------------------------------------------------------
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
  display.clearDisplay();
  display.setTextSize(2);             
  display.setTextColor(WHITE); 
  //display.setCursor(0,20);  
}
///******************************************************************************************
void loop()
{
    
   // switchStatus1 = digitalRead(switch1_pin);   // read status of switch
    //if (switchStatus1 != switchStatus1_Last)  // if status of button has changed
   //   {
   //     // if switch is pressed than change the LED status
   //     if (switchStatus1 == HIGH && switchStatus1_Last == LOW) 
   //     {
   //       sens_count=sens_count+1;
   //       onBeatDetected();
   //     }
   //     switchStatus1 = switchStatus1_Last;
   //   }
    
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
    {
      PulseOxi();
      
      tsLastReport = millis();
    }
}

