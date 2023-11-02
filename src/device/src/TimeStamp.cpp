#include <sti/utils/TimeStamp.h>

#include <sti/utils/utils.h>

#include <chrono>
#include <sstream>
#include <iomanip>

#include "CerealArchives.h"

using STI::Utils::TimeStamp;


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

void TimeStamp::add_day(int days)
{
    timeinfo.tm_mday += days;
}

void TimeStamp::add_hour(int hours)
{
    timeinfo.tm_hour += hours;
}

void TimeStamp::add_minute(int minutes)
{
    timeinfo.tm_min += minutes;
}

void TimeStamp::add_sec(int seconds)
{
    timeinfo.tm_sec += seconds;
}

void TimeStamp::add_ns(int ns)
{
    _nanos += ns;
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
    return date_YYYY_MM_DD("/");
}

std::string TimeStamp::date_YYYY_MM_DD(const std::string& separator) const
{
    std::stringstream ts;
    
    ts << year();
    ts << separator;
    ts << std::setfill('0') << std::setw(2) << month();
    ts << separator;
    ts << std::setfill('0') << std::setw(2) << day();

    return ts.str();
}

std::string TimeStamp::time() const
{
    std::stringstream ts;

    ts << time_hh_mm_ss(":");
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
    return time_hh_mm_ss("_");
}

std::string TimeStamp::time_hh_mm_ss(const std::string& separator) const
{
    std::stringstream ts;

    ts << std::setfill('0') << std::setw(2) << hour();
    ts << separator;
    ts << std::setfill('0') << std::setw(2) << minute();
    ts << separator;
    ts << std::setfill('0') << std::setw(2) << sec();

    return ts.str();
}


std::string TimeStamp::time_mmmuuunnn(const std::string& separator) const
{
    std::stringstream ts;

    ts << std::setfill('0') << std::setw(3) << millis();
    ts << separator;
    ts << std::setfill('0') << std::setw(3) << micros();
    ts << separator;
    ts << std::setfill('0') << std::setw(3) << nanos();

    return ts.str();
}


std::string TimeStamp::time_hh_mm_ss_mmmuuunnn() const
{
    std::stringstream ts;
    
    ts << time_hh_mm_ss();
    ts << "_";
    ts << time_mmmuuunnn("");
    // ts << std::setfill('0') << std::setw(3) << millis();
    // ts << std::setfill('0') << std::setw(3) << micros();
    // ts << std::setfill('0') << std::setw(3) << nanos();

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

std::string TimeStamp::toString() const
{
    std::stringstream ts;

    //2023/09/15|11:45:09.235.567.129

    ts << date_YYYY_MM_DD("/") << "|" << time_hh_mm_ss(":") << "." << time_mmmuuunnn(".");

    return ts.str();
}

TimeStamp TimeStamp::fromString(const std::string& timeStamp)
{
    using STI::Utils::stringToValue;

    std::vector<std::string> tokens;
    STI::Utils::splitString(timeStamp, "|", tokens);    // date, time

    if (tokens.size() == 0 || tokens.size() > 2) return TimeStamp();

    std::string date;
    std::string time;

    if (tokens.size() >= 1) {
        //assume date
        date = tokens.at(0);
    }
    if (tokens.size() == 2) {
        //time
        time = tokens.at(1);
    }

    int year = 0;
    int month = 0;
    int day = 0;

    STI::Utils::splitString(date, "/", tokens);     //YYYY, MM, DD
    bool validDate = (tokens.size() == 3) ||
        (tokens.size() == 4 && tokens.at(3) == "");
    if (!validDate) return TimeStamp();     //invalid date
    
    bool success = 
        stringToValue(tokens.at(0), year) && 
        stringToValue(tokens.at(1), month) && 
        stringToValue(tokens.at(2), day);

    if (!success) return TimeStamp();   //invalid date

    STI::Utils::splitString(time, ".", tokens); // hh:mm::ss, mmm, uuu, nnn
    if (tokens.size() == 0 || tokens.size() > 4) return TimeStamp(year, month, day, 0, 0, 0, 0, 0, 0);   //missing valid time

    std::string hhmmss, mmm, uuu, nnn;

    if (tokens.size() > 0) {
        hhmmss = tokens.at(0);
    }
    if (tokens.size() > 1) {
        mmm = tokens.at(1);
    }
    if (tokens.size() > 2) {
        uuu = tokens.at(2);
    }
    if (tokens.size() == 4) {
        nnn = tokens.at(3);
    }

    int ms = 0;
    int us = 0;
    int ns = 0;

    if (!stringToValue(mmm, ms)) { ms = 0; }
    if (!stringToValue(uuu, us)) { us = 0; }
    if (!stringToValue(nnn, ns)) { ns = 0; }

    STI::Utils::splitString(hhmmss, ":", tokens);     // hh, mm, ss
    if (tokens.size() != 3) return TimeStamp(year, month, day, 0, 0, 0, 0, 0, 0);   //invalid time

    int hour = 0;
    int minute = 0;
    int second = 0;

    success = 
        stringToValue(tokens.at(0), hour) && 
        stringToValue(tokens.at(1), minute) && 
        stringToValue(tokens.at(2), second);

    if (!success) return TimeStamp(year, month, day, 0, 0, 0, 0, 0, 0);   //invalid time

    TimeStamp ts(year, month, day, hour, minute, second, ms, us, ns);
    return ts;
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

bool TimeStamp::operator<=(const TimeStamp& rhs) const
{
    return (*this) < rhs || (*this) == rhs;
}

bool TimeStamp::isSameDate(const TimeStamp& rhs) const
{
    return day() == rhs.day() &&
           month() == rhs.month() &&
           year() == rhs.year();
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
template void STI::Utils::TimeStamp::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Utils::TimeStamp::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void STI::Utils::TimeStamp::serialize<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& );
template void STI::Utils::TimeStamp::serialize<cereal::JSONInputArchive>( cereal::JSONInputArchive& );
