#include "Wecker.h"
#include <Preferences.h>
#include "DaylightTime.h"
#include "TimeHelper.h"

extern DaylightTime dst; // Zugriff auf DST-Objekt aus main.cpp

Wecker::Wecker()
{
    weckStunde = 7;
    weckMinute = 0;
    active = false;
}

void Wecker::setWeckzeit(int stunde, int minute)
{
    weckStunde = stunde;
    weckMinute = minute;
}

void Wecker::getWeckzeit(int &stunde, int &minute)
{
    stunde = weckStunde;
    minute = weckMinute;
}

void Wecker::setActive(bool state)
{
    active = state;
}

bool Wecker::isActive()
{
    return active;
}

void Wecker::saveToPreferences()
{
    Preferences prefs;
    prefs.begin("wecker", false);
    prefs.putInt("stunde", weckStunde);
    prefs.putInt("minute", weckMinute);
    prefs.putBool("active", active);
    prefs.end();
}

void Wecker::loadFromPreferences()
{
    Preferences prefs;
    prefs.begin("wecker", true);
    weckStunde = prefs.getInt("stunde", 7);
    weckMinute = prefs.getInt("minute", 0);
    active = prefs.getBool("active", false);
    prefs.end();
}

bool Wecker::shouldTriggerAlarm()
{
    static bool alreadyTriggered = false;

    if (!active)
        return false;

    struct tm timeinfo;
    if (getCorrectedLocalTime(timeinfo))
    {
        if (timeinfo.tm_hour == weckStunde && timeinfo.tm_min == weckMinute)
        {
            if (!alreadyTriggered)
            {
                alreadyTriggered = true;
                return true;
            }
        }
        else
        {
            alreadyTriggered = false; // Reset, wenn Zeit vorbei
        }
    }
    return false;
}
