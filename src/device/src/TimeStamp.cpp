
#include "TimeStamp.h"

#include <chrono>
#include <sstream>
#include <iomanip>

using STI::Engine::TimeStamp;

TimeStamp::TimeStamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    timeinfo = *localtime(&now_time);

    auto duration = now.time_since_epoch();

    auto duration_secs   = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto duration_millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto duration_micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    auto duration_nanos  = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    _millis = static_cast<int>(duration_millis - (1000*duration_secs));
    _micros = static_cast<int>(duration_micros - (1000*duration_millis));
    _nanos  = static_cast<int>(duration_nanos - (1000*duration_micros));
}

TimeStamp::TimeStamp(int year, int month, int day, int hour,
            int min, int sec, int millis, int micros, int nanos)
: _millis(millis), _micros(micros), _nanos(nanos)
{
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;

    std::time_t now_time = mktime(&timeinfo);
    timeinfo = *localtime(&now_time);
}

int TimeStamp::year() const
{
    return 1900 + timeinfo.tm_year;
}
int TimeStamp::month() const
{
    return 1 + timeinfo.tm_mon;
}
int TimeStamp::day() const
{
    return timeinfo.tm_mday;
}
int TimeStamp::hour() const
{
    return timeinfo.tm_hour;
}
int TimeStamp::min() const
{
    return timeinfo.tm_min;
}
int TimeStamp::sec() const
{
    return timeinfo.tm_sec;
}
int TimeStamp::millis() const
{
    return _millis;
}
int TimeStamp::micros() const
{
    return _micros;
}
int TimeStamp::nanos() const
{
    return _nanos;
}

std::string TimeStamp::getMonthName() const
{
	char buffer[50];
    std::string name;

    std::strftime(buffer, sizeof(buffer), "%b", &timeinfo);
    name = buffer;

    return name;
}

std::string TimeStamp::getDayName() const
{
	char buffer[50];
    std::string name;

    std::strftime(buffer, sizeof(buffer), "%a", &timeinfo);
    name = buffer;

    return name;
}

std::string TimeStamp::date() const
{
    std::stringstream ts;

    ts << getDayName() << " " << getMonthName() 
       << " " << day() << " " << year();

    return ts.str();
}

std::string TimeStamp::date_YYYY_MM_DD() const
{
    std::stringstream ts;
    
    ts << year();
    ts << "/";
    ts << std::setfill('0') << std::setw(2) << month();
    ts << "/"; 
    ts << std::setfill('0') << std::setw(2) << day();

    return ts.str();
}

std::string TimeStamp::time() const
{
    std::stringstream ts;

    ts << std::setfill('0') << std::setw(2) << hour();
    ts << ":";
    ts << std::setfill('0') << std::setw(2) << min();
    ts << ":";
    ts << std::setfill('0') << std::setw(2) << sec();
    ts << ".";

    ts << std::setfill('0') << std::setw(3) << millis();
    ts << " ";
    ts << std::setfill('0') << std::setw(3) << micros();
    ts << " ";
    ts << std::setfill('0') << std::setw(3) << nanos();

    return ts.str();
}

std::string TimeStamp::print() const
{
    std::stringstream ts;

    ts << date();

    ts << " (" << date_YYYY_MM_DD() << ") ";

    ts << time();

    return ts.str();
}

bool TimeStamp::operator<(const TimeStamp& rhs) const
{
    if (year() < rhs.year())
        return true;
    else if (year() > rhs.year())
        return false;

    if (month() < rhs.month())
        return true;
    else if (month() > rhs.month())
        return false;

    if (day() < rhs.day())
        return true;
    else if (day() > rhs.day())
        return false;

    if (hour() < rhs.hour())
        return true;
    else if (hour() > rhs.hour())
        return false;
    
    if (min() < rhs.min())
        return true;
    else if (min() > rhs.min())
        return false;

    if (sec() < rhs.sec())
        return true;
    else if (sec() > rhs.sec())
        return false;

    if (millis() < rhs.millis())
        return true;
    else if (millis() > rhs.millis())
        return false;

    if (micros() < rhs.micros())
        return true;
    else if (micros() > rhs.micros())
        return false;

    return nanos() < rhs.nanos(); 
}

bool TimeStamp::operator==(const TimeStamp& rhs) const
{
    return nanos() == rhs.nanos() &&
           micros() == rhs.micros() &&
           millis() == rhs.millis() &&
           sec() == rhs.sec() &&
           min() == rhs.min() &&
           hour() == rhs.hour() &&
           day() == rhs.day() &&
           month() == rhs.month() &&
           year() == rhs.year();
}

bool TimeStamp::operator!=(const TimeStamp& rhs) const
{
    return !((*this) == rhs);
}

