#include "DaylightTime.h"

DaylightTime::DaylightTime() : startDay(0), endDay(0), storedYear(0) {}

int DaylightTime::getLastSunday(int year, int month)
{
    int day = 31;
    while (true)
    {
        int y = (month < 3) ? year - 1 : year;
        int m = (month < 3) ? month + 12 : month;
        int k = y % 100;
        int j = y / 100;
        int h = (day + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
        int weekday = ((h + 5) % 7) + 1; // 1=Montag, 7=Sonntag
        if (weekday == 7)
            break; // Sonntag gefunden
        day--;
    }
    return day;
}

void DaylightTime::calculateDST(int year)
{
    startDay = getLastSunday(year, 3);
    endDay = getLastSunday(year, 10);
    storedYear = year;
}

int DaylightTime::getOffset(int year, int month, int day, int hour)
{
    if (year != storedYear)
    {
        calculateDST(year);
    }

    if ((month > 3 && month < 10) ||
        (month == 3 && (day > startDay || (day == startDay && hour >= 2))) ||
        (month == 10 && (day < endDay || (day == endDay && hour < 3))))
    {
        return 3600; // Sommerzeit
    }
    return 0; // Winterzeit
}
