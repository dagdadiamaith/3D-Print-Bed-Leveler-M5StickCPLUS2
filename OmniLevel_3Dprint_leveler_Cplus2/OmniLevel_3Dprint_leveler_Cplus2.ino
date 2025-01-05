/* 3D Printer Bed Leveler
 * Copyright (C) 2020 by Dominick Lee (http://dominicklee.com)
 * 
 * Last Modified Jun, 2024 by DagdaDiamaith.
 * This program is free software: you can use it, redistribute it, or modify
 * it under the terms of the MIT license (See LICENSE file for details).
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
 * ANY CLAIM, DAMAGES,OR OTHER LIABILITY. YOU AGREE TO USE AT YOUR OWN RISK.
 * 
*/

/*
I modified this program, in order to easily run on M5StickCplus2.
Do not forget to load in the Arduino editor to set the card type to use M5StickCplus2.
*/


// for M5StickCplus2
// Begin M5StickCplus2 bloc
#include <M5StickCPlus2.h>
#include <EEPROM.h>
#include "OneButton.h"
#include "rotate-leftPlus.h"
#include "rotate-rightPlus.h"
#include "thumbs-upPlus.h"
int set_point = 92;  // global
int rect_width = 45;
int rect_height = 190;
int x_off = 60;
int y_off = 113;
int x_cursor = 20;
int y_cursor = 220;
int pressure_out = 184;
int text_size = 2;
int x_image = 70;
int x_size_image = 48;
int y_size_image = 48;
int x_calibration = 50;
int x_calibration_end = 54;
int x_progressbar = 199;
int x_progressbar_width = 40;
int adjust_color = 46;
// end M5StickCPlus2 bloc

// Icons converted at:
// http://rinkydinkelectronics.com/_t_doimageconverter565.php

#define EEPROM_SIZE 1
#define LED_PIN 19  // Définir manuellement la broche 19 pour le LED
//int LED_PIN = G19;
int pressure = 0;            //the var that goes between 0-127 from FSR reading
int setpoint = set_point;    //to be updated by EEPROM
float errorThreshold = 6.0;  //printbed is considered leveled if error goes below this (ideally 5.0-8.0)

int shiftX = 10;
int shiftY = 10;

//Object initialization
OneButton btnA(G37, true);
OneButton btnB(G39, true);

void setup() {
  M5.begin();
  EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
  EEPROM.write(0, setpoint);  // save in EEPROM
  EEPROM.commit();
  Serial.begin(115200);        //Initialize Serial
  pinMode(LED_PIN, OUTPUT);     //Set up LED
  digitalWrite(LED_PIN, HIGH);  // turn off the LED

  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(BLACK);

  btnA.attachClick(btnAClick);  //BtnA handle
  btnA.setDebounceMs(40);
  btnB.attachClick(btnBClick);  //BtnB handle
  btnB.setDebounceMs(25);

  M5.Lcd.drawRect(shiftX + 5, 12, 21, 133, 0x7bef);  //show frame for progressbar (X, Y Width, height, color)
  getCalibration();                                  //show calibration mark

  //Show OFF instructions
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(85, 105);
  M5.Lcd.printf("Off ->");

  //Show Calibrate instructions
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(65, 80);
  M5.Lcd.printf("Calibrate ->");

  // Swap the colour byte order when rendering
  M5.Lcd.setSwapBytes(true);
}

void loop() {
  //poll for button press
  btnA.tick();
  btnB.tick();

  //pressure = map(analogRead(G26), 0, 4095, 0, 127); //get reading input number, begin range read, end range read, begin range output, end range output // old line
  pressure = map(analogRead(G26), 0, 4095, 0, pressure_out);  //get reading input number, begin range read, end range read, begin range output, end range output
  Serial.println(pressure);

  progressBar(pressure);  //show reading on progressbar

  if (getPercentError(pressure, setpoint) < errorThreshold) {
    setLED(true);
    // M5.Lcd.pushImage(40, 15, 32, 32, thumbs_up);  //Draw icon position X, Y ,image size x, y // old line
    M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, thumbs_up);  //Draw icon position X, Y ,image size x, y
  } else {
    setLED(false);
    if (pressure > setpoint) {
      // M5.Lcd.pushImage(40, 15, 32, 32, rotate_left);  //Draw icon position X, Y ,image size x, y // old line
      M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, rotate_left);  //Draw icon position X, Y ,image size x, y
    } else {
      // M5.Lcd.pushImage(40, 15, 32, 32, rotate_right);  //Draw icon position X, Y ,image size x, y // old line
      M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, rotate_right);  //Draw icon position X, Y ,image size x, y
    }
  }
  delay(50);
}

void btnBClick() {
  M5.Power.powerOff();
}

void btnAClick() {
  updateCalibration(pressure);
}

float getPercentError(float approx, float exact) {
  return (abs(approx - exact) / exact) * 100;
}

void updateCalibration(int value) {
  EEPROM.write(0, value);  // save in EEPROM
  EEPROM.commit();
  // clear old line
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), BLACK);  // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), BLACK); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15 + (pressure_out - setpoint), 4, 15 + (pressure_out - setpoint), BLACK);                              // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15 + (pressure_out - setpoint), x_calibration_end, 15 + (pressure_out - setpoint), BLACK);  // draw line from X, Y to x, y
  // set new line
  setpoint = value;  //set global
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15 + (pressure_out - setpoint), 4, 15 + (pressure_out - setpoint), 0x7bef);                              // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15 + (pressure_out - setpoint), x_calibration_end, 15 + (pressure_out - setpoint), 0x7bef);  // draw line from X, Y to x, y
}

void getCalibration() {
  setpoint = EEPROM.read(0);  // retrieve calibration in EEPROM
  // set new line
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15 + (pressure_out - setpoint), 4, 15 + (pressure_out - setpoint), 0x7bef);                              // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15 + (pressure_out - setpoint), x_calibration_end, 15 + (pressure_out - setpoint), 0x7bef);  // draw line from X, Y to x, y
}

void setLED(bool isON) {
  digitalWrite(LED_PIN, !isON);  // set the LED
}

void progressBar(int value) {
  // Value is expected to be in range 0-127
  for (int i = 0; i <= value; i++) {  //draw bar
    // M5.Lcd.fillRect(8, 142-i, 15, 1, rainbow(i)); // X, Y, width, height from rectangle to fill, if Height = 1 its like a line. // old line
    M5.Lcd.fillRect(8, x_progressbar - i, x_progressbar_width, 1, rainbow(i));  // X, Y, width, height from rectangle to fill, if Height = 1 its like a line.
  }

  // for (int i = value+1; i <= 128; i++) {  //clear old stuff old line
  for (int i = value + 1; i <= (pressure_out + 1); i++) {  //clear old stuff
    // M5.Lcd.fillRect(8, 142-i, 15, 1, BLACK); // clean rectangle with black // old line
    M5.Lcd.fillRect(8, x_progressbar - i, x_progressbar_width, 1, BLACK);  // clean rectangle with black
  }
}

unsigned int rainbow(int value) {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to red = blue
  //int value = random (128);
  byte red = 0;    // Red is the top 5 bits of a 16 bit colour value 1111 1000 0000 0000 63 level of red
  byte green = 0;  // Green is the middle 6 bits 0000 0111 1110 0000 63 level of green
  byte blue = 0;   // Blue is the bottom 5 bits 0000 0000 0001 1111 31 level of blue

  byte quadrant = value / adjust_color;
  int valb = map(value, 0, pressure_out, 0, 127);
  if (quadrant == 0) {
    blue = 31;
    green = 2 * (valb % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - 1 - (valb % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = valb % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (valb % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// Old version
/*unsigned int rainbow(int value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to red = blue
  //int value = random (128);
  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;
    int valb = value;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}*/
