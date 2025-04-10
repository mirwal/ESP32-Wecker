#include "DisplayHelper.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

DisplayHelper::DisplayHelper() : lcd(0x27, 20, 4) {}
String DisplayHelper::padToLength(const String &input, int length)
{
    String result = input;
    while (result.length() < length)
        result += ' ';
    return result;
}
void DisplayHelper::begin()
{
    lcd.init();
    lcd.backlight();
    defineCustomUmlauts(); // ← Umlaute registrieren
    for (int i = 0; i < 4; i++)
    {
        lines[i] = "";
        lastLines[i] = "";
    }
}
String padRight(const String &input, size_t length)
{
    String result = input;
    while (result.length() < length)
    {
        result += ' ';
    }
    if (result.length() > length)
    {
        result = result.substring(0, length);
    }
    return result;
}

void DisplayHelper::setLine(uint8_t line, const String &text)
{
    if (line < 4)
    {
        lines[line] = text; // Nur speichern – Padding macht printWithUmlaut()
        scroll[line].enabled = false;
    }
}

void DisplayHelper::setScrollingLine(uint8_t line, const String &text, unsigned long delay)
{
    if (line < 4)
    {
        scroll[line].text = text + "    ";
        scroll[line].pos = 0;
        scroll[line].delay = delay;
        scroll[line].lastUpdate = millis();
        scroll[line].enabled = true;
    }
}

void DisplayHelper::setScrollingLineWords(uint8_t line, const String &text, unsigned long waitTime)
{
    if (line >= 4)
        return;

    // Text in Wörter zerlegen
    std::vector<String> words;
    int start = 0;
    while (start < text.length())
    {
        int end = text.indexOf(' ', start);
        if (end == -1)
            end = text.length();
        words.push_back(text.substring(start, end));
        start = end + 1;
    }

    scroll[line].wordList = words;
    scroll[line].wordIndex = 0;
    scroll[line].clearShown = false;
    scroll[line].delay = waitTime;
    scroll[line].lastUpdate = 0;
    scroll[line].enabled = true;
    scroll[line].isWordScroll = true;
}

void DisplayHelper::clearScrolling(uint8_t line)
{
    if (line < 4)
    {
        scroll[line].enabled = false;
    }
}

String DisplayHelper::getWordScrollView(uint8_t line)
{
    unsigned long now = millis();
    const int maxLen = 20;

    // Noch nicht bereit für ein Update
    if (now - scroll[line].lastUpdate < scroll[line].delay)
    {
        return lastLines[line]; // Zeige alten Inhalt
    }

    scroll[line].lastUpdate = now;

    String displayText = "";
    int i = scroll[line].wordIndex;

    // Ende erreicht → einmal leer anzeigen
    if (i >= scroll[line].wordList.size())
    {
        if (!scroll[line].clearShown)
        {
            scroll[line].clearShown = true;
            return String(' ', maxLen); // Leere Zeile mit 20 Leerzeichen
        }
        else
        {
            scroll[line].wordIndex = 0;
            scroll[line].clearShown = false;
        }
    }

    // Wörter zusammensetzen bis die Zeile voll ist
    while (i < scroll[line].wordList.size())
    {
        String next = (displayText.length() == 0)
                          ? scroll[line].wordList[i]
                          : displayText + " " + scroll[line].wordList[i];
        if (next.length() > maxLen)
            break;
        displayText = next;
        i++;
    }

    // Immer auf 20 Zeichen auffüllen (wichtig!)
    if (displayText.length() < maxLen)
    {
        // displayText += String(' ', maxLen - displayText.length());
        displayText = padToLength(displayText, maxLen);
    }
    else if (displayText.length() > maxLen)
    {
        displayText = padToLength(displayText.substring(0, maxLen), maxLen); // safety
    }

    scroll[line].wordIndex = i; // 🔧 ← Das war das fehlende Zahnrad!
    return displayText;
}

String DisplayHelper::getScrollView(uint8_t line)
{
    // Beispielimplementierung
    String fullText = scroll[line].text;
    int pos = scroll[line].pos;
    int width = 20;

    // Rolltext berechnen
    String view = fullText.substring(pos, pos + width);
    if (view.length() < width)
        view += fullText.substring(0, width - view.length());

    // Position aktualisieren
    unsigned long now = millis();
    if (now - scroll[line].lastUpdate >= scroll[line].delay)
    {
        scroll[line].pos = (scroll[line].pos + 1) % fullText.length();
        scroll[line].lastUpdate = now;
    }

    return view;
}

void DisplayHelper::defineCustomUmlauts()
{
    // https://maxpromer.github.io/LCD-Character-Creator/
    byte aeChar[8] = {
        B01010, B00000, B01110, B00001, B01111, B10001, B01111, B00000};
    byte oeChar[8] = {
        B01010, B00000, B01110, B10001, B10001, B10001, B01110, B00000};
    byte ueChar[8] = {
        B01010, B00000, B10001, B10001, B10001, B10011, B01101, B00000};
    byte ssChar[8] = {
        B00000, B00000, B01100, B10010, B10100, B10010, B10100, B10000};

    lcd.createChar(0, aeChar);
    lcd.createChar(1, oeChar);
    lcd.createChar(2, ueChar);
    lcd.createChar(3, ssChar);
}

void DisplayHelper::printWithUmlaut(const String &text)
{
    uint8_t col = 0;
    int i = 0;

    while (col < 20 && i < text.length())
    {
        uint8_t c = text[i];

        if (c == 0x00)
        {
            lcd.write(0);
            i++;
            col++;
            continue;
        } // ä
        if (c == 0x01)
        {
            lcd.write(1);
            i++;
            col++;
            continue;
        } // ö
        if (c == 0x02)
        {
            lcd.write(2);
            i++;
            col++;
            continue;
        } // ü
        if (c == 0x03)
        {
            lcd.write(3);
            i++;
            col++;
            continue;
        } // ß

        if (i + 1 < text.length() && (uint8_t)c == 0xC3)
        {
            uint8_t next = text[i + 1];

            if (next == 0xA4)
            {
                lcd.write(0);
                i += 2;
                col++;
            }
            else if (next == 0xB6)
            {
                lcd.write(1);
                i += 2;
                col++;
            }
            else if (next == 0xBC)
            {
                lcd.write(2);
                i += 2;
                col++;
            }
            else if (next == 0x9F)
            {
                lcd.write(3);
                i += 2;
                col++;
            }
            else
            {
                lcd.print((char)c);
                i++;
                col++;
            }
        }
        else
        {
            lcd.print((char)c);
            i++;
            col++;
        }
    }

    while (col < 20)
    {
        lcd.print(" ");
        col++;
    }
}

void DisplayHelper::show()
{
    for (int i = 0; i < 4; i++)
    {
        String displayText;
        if (scroll[i].enabled)
        {
            if (scroll[i].isWordScroll)
            {
                // Wortweise Darstellung
                displayText = getWordScrollView(i); // neue Methode
            }
            else
            {
                // Zeichenweises Scrolling
                displayText = getScrollView(i);
            }
        }
        else
        {
            displayText = lines[i]; // Feste Zeile
        }

        if (displayText != lastLines[i])
        {
            lcd.setCursor(0, i);
            printWithUmlaut(displayText);
            lastLines[i] = displayText;
            Serial.println("-" + displayText + "-");
        }
    }
}
