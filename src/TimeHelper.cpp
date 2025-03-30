#include <Arduino.h>
#include "TimeHelper.h"
#include "DaylightTime.h"

#include <time.h>
#include <sys/time.h>

extern DaylightTime dst;

bool getCorrectedLocalTime(struct tm &timeinfo)
{
    if (getLocalTime(&timeinfo))
    {
        int offset = dst.getOffset(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour);
        time_t now = mktime(&timeinfo) + offset;
        struct tm *localTime = localtime(&now);
        timeinfo = *localTime;
        return true;
    }
    return false;
}
