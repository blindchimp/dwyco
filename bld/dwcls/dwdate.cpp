#include "dwdate.h"

DwDate::DwDate()
{
    clk = time(0);
    stm = *localtime(&clk);
}

DwDate::DwDate(time_t t)
{
    clk = t;
    stm = *localtime(&clk);
}

int
DwDate::Year()
{
    return stm.tm_year + 1900;
}

int
DwDate::Day()
{
    return stm.tm_yday;
}

int
DwDate::DayOfMonth()
{
    return stm.tm_mday;
}

int
DwDate::Month()
{
    return stm.tm_mon + 1;
}

DwTime::DwTime()
{
    clk = time(0);
    stm = *localtime(&clk);
}

DwTime::DwTime(int hour, int minute, int second)
{
    clk = time(0);
    stm = *localtime(&clk);
    stm.tm_sec = second;
    stm.tm_hour = hour;
    stm.tm_min = minute;
}

DwString
DwTime::AsString()
{
    char a[200];
    memset(a, 0, sizeof(a));
#if defined(__BORLANDC__) || defined(_MSC_VER)
    strftime(a, sizeof(a) - 1, "%c", &stm);
#else
    strftime(a, sizeof(a) - 1, "%T", &stm);
#endif
    return DwString(a);
}

int
DwTime::Hour()
{
    return stm.tm_hour;
}

int
DwTime::Minute()
{
    return stm.tm_min;
}

int
DwTime::Second()
{
    return stm.tm_sec;
}
void
DwTime::PrintDate(int)
{
}
