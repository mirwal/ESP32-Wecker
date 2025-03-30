#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class DisplayHelper
{
public:
    DisplayHelper(LiquidCrystal_I2C &lcd) : lcd(lcd) {}
    void begin()
    {
        lcd.init();
        lcd.backlight();
    }
    void setLine(int line, const String &text)
    {
        if (line >= 0 && line < 4)
        {
            String t = text; // Kopie
            // Text auf genau 20 Zeichen anpassen
            if (t.length() > 20)
            {
                t = t.substring(0, 20);
            }
            else
            {
                while (t.length() < 20)
                {
                    t += " ";
                }
            }
            lines[line] = t;
        }
    }

    void show()
    {

        for (int i = 0; i < 4; i++)
        {
            // lcd.setCursor(0, i);
            // lcd.print("                    ");
            lcd.setCursor(0, i);
            lcd.print(lines[i]);
        }
    }

    void clear()
    {
        for (int i = 0; i < 4; i++)
        {
            lines[i] = "";
        }
        lcd.clear();
    }

private:
    LiquidCrystal_I2C &lcd;
    String lines[4] = {"", "", "", ""};
};