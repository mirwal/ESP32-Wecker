#pragma once
#include <Arduino.h>
#include <vector>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class DisplayHelper
{
public:
    DisplayHelper();
    void begin();
    void setLine(uint8_t line, const String &text);
    void setScrollingLine(uint8_t line, const String &text, unsigned long delay = 300);
    void setScrollingLineWords(uint8_t line, const String &text, unsigned long waitTime);
    String getWordScrollView(uint8_t line);
    void clearScrolling(uint8_t line);
    void show();

private:
    LiquidCrystal_I2C lcd;
    String lines[4];
    String lastLines[4];
    String padToLength(const String &input, int length);
    struct ScrollData
    {
        String text;
        size_t pos = 0;
        unsigned long lastUpdate = 0;
        unsigned long delay = 300;
        bool enabled = false;

        // Für wortweises Scrolling:
        std::vector<String> wordList;
        int wordIndex = 0;
        bool isWordScroll = false;
        bool clearShown = false;
    };

    ScrollData scroll[4];
    void defineCustomUmlauts();
    void printWithUmlaut(const String &text);
    String getScrollView(uint8_t line);
};
