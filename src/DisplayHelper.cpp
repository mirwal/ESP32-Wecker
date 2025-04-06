#include "DisplayHelper.h"
#include <Arduino.h>
DisplayHelper::DisplayHelper(LiquidCrystal_I2C &lcd) : lcd(lcd) {}

void DisplayHelper::begin()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
    for (int i = 0; i < 4; i++)
    {
        lines[i] = "";
        lastLines[i] = "";
    }
}

void DisplayHelper::setLine(uint8_t line, const String &text)
{
    if (line < 4)
    {
        lines[line] = text;
    }
}

void DisplayHelper::show()
{
    for (int i = 0; i < 4; i++)
    {
        String displayText = lines[i];
        while (displayText.length() < 20)
            displayText += ' '; // mit Leerzeichen auffüllen

        if (displayText != lastLines[i])
        {
            lcd.setCursor(0, i);
            lcd.print(displayText);
            lastLines[i] = displayText; // mit Leerzeichen merken!
            Serial.println("-" + displayText + "-");
        }
    }
}
