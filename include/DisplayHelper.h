#ifndef DISPLAYHELPER_H
#define DISPLAYHELPER_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class DisplayHelper
{
public:
    DisplayHelper(LiquidCrystal_I2C &lcd);
    void begin();
    void setLine(uint8_t line, const String &text);
    void show();

private:
    LiquidCrystal_I2C &lcd;
    String lines[4];     // gewünschter Inhalt
    String lastLines[4]; // zuletzt gezeigter Inhalt
};
#endif