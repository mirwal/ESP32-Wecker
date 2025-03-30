#ifndef TIMEHELPER_H
#define TIMEHELPER_H

#include <time.h>

#include <sys/time.h>
bool getCorrectedLocalTime(struct tm &timeinfo);

#endif