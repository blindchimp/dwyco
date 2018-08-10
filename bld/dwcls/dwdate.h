#ifndef DWDATE_H
#define DWDATE_H
// hack that mimics the old borland classlib TDate/TTime class
// roughly.

#include "dwstr.h"
#include <time.h>

class DwDate
{
public:
    DwDate();
    DwDate(time_t);
    //DwDate(int jday, int year);

    int Year();
    int Day();
    int DayOfMonth();
    int Month();
private:
    struct tm stm;
    time_t clk;
};

class DwTime
{
public:
    DwTime();
    DwTime(int hour, int minute, int second);

    int Hour();
    int Minute();
    int Second();
    DwString AsString();
    static void PrintDate(int);
private:
    struct tm stm;
    time_t clk;
};

#endif
