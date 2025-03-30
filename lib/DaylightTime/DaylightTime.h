#ifndef DAYLIGHTTIME_H
#define DAYLIGHTTIME_H

class DaylightTime
{
public:
    DaylightTime();
    int getOffset(int year, int month, int day, int hour);

private:
    int startDay;
    int endDay;
    int storedYear;
    void calculateDST(int year);
    int getLastSunday(int year, int month);
};

#endif
