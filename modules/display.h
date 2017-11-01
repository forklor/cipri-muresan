#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C display_lcd;

void _display_setup();
void _display_loop(long milliseconds);

#endif //DISPLAY_H_