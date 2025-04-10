#ifndef WECKER_H
#define WECKER_H

#include <Arduino.h>
#include <time.h>

class Wecker
{
public:
    Wecker();
    void setWeckzeit(int stunde, int minute);
    void getWeckzeit(int &stunde, int &minute);
    void setActive(bool active);
    bool isActive();
    void saveToPreferences();
    void loadFromPreferences();
    bool shouldTriggerAlarm();
    uint8_t getSavedVolume();
    void setSavedVolume(uint8_t volume);
    uint8_t getSavedSound();
    void setSavedSound(uint8_t sound);

private:
    int weckStunde;
    int weckMinute;
    bool active;
};

#endif