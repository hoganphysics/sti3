#include <sti/engine/TimeStamp.h>

#include <chrono>
#include <sstream>
#include <iomanip>
//#include <time.h>

#include "CerealArchives.h"

using STI::Engine::TimeStamp;


TimeStamp::TimeStamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    timeinfo = *localtime(&now_time);
//    localtime_s(&timeinfo, &now_time);

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
    timeinfo.tm_isdst = -1; //use local system timezone (mktime will fill this in)

    std::time_t now_time = mktime(&timeinfo);
    
    //timeinfo = *localtime(&now_time);   //  1/2/2023
    //localtime_s(&timeinfo, &now_time);      //this is working in windows 1/21/2023

    //generate local time using timeinfo (retrieves dst information)
    auto generatedLocalTime = localtime(&now_time);
    
    //null check here because localtime returns null when now_time=-1, which can indicate some
    //illegal values in timeinfo (such as when TimeStamp is improperly initialized)
    if (generatedLocalTime != 0) {
        timeinfo = *generatedLocalTime;     //update timeinfo with validated info
    }
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
int TimeStamp::minute() const
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
    ts << std::setfill('0') << std::setw(2) << minute();
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

std::string TimeStamp::time_hh_mm_ss() const
{
    std::stringstream ts;

    ts << std::setfill('0') << std::setw(2) << hour();
    ts << "_";
    ts << std::setfill('0') << std::setw(2) << minute();
    ts << "_";
    ts << std::setfill('0') << std::setw(2) << sec();

    return ts.str();
}

std::string TimeStamp::time_hh_mm_ss_mmmuuunnn() const
{
    std::stringstream ts;
    
    ts << time_hh_mm_ss();
    ts << "_";
    ts << std::setfill('0') << std::setw(3) << millis();
    ts << std::setfill('0') << std::setw(3) << micros();
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
    
    if (minute() < rhs.minute())
        return true;
    else if (minute() > rhs.minute())
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
           minute() == rhs.minute() &&
           hour() == rhs.hour() &&
           day() == rhs.day() &&
           month() == rhs.month() &&
           year() == rhs.year();
}

bool TimeStamp::operator!=(const TimeStamp& rhs) const
{
    return !((*this) == rhs);
}


template<class Archive>
void serialize(Archive& archive, tm& timeinfo)
{
    archive(cereal::make_nvp("sec", timeinfo.tm_sec),
            cereal::make_nvp("min", timeinfo.tm_min),
            cereal::make_nvp("hour", timeinfo.tm_hour),
            cereal::make_nvp("mday", timeinfo.tm_mday),
            cereal::make_nvp("mon", timeinfo.tm_mon),
            cereal::make_nvp("year", timeinfo.tm_year),
            cereal::make_nvp("wday", timeinfo.tm_wday),
            cereal::make_nvp("year", timeinfo.tm_yday),
            cereal::make_nvp("isdst", timeinfo.tm_isdst));
}

template<class Archive>
void TimeStamp::serialize(Archive& archive)
{
    
    archive(cereal::make_nvp("timeinfo", timeinfo), 
            cereal::make_nvp("millis", _millis), 
            cereal::make_nvp("micros", _micros), 
            cereal::make_nvp("nanos", _nanos));//timeinfo, 
}

// Note that we need to instantiate for both loading and saving, even
// if we use a single serialize function
template void STI::Engine::TimeStamp::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::TimeStamp::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
