#ifndef STI_ENGINE_TIMESTAMP_H
#define STI_ENGINE_TIMESTAMP_H

#include <string>


namespace STI
{
namespace Utils
{


class TimeStamp
{
public:
	
	TimeStamp();
	TimeStamp(int year, int month, int day, int hour,
    		  int min, int sec, int millis, int micros, int nanos);

	//date
	//time
	//timezone
	// double timestamp;	//show use std::chrono
	std::string print() const;

	std::string date() const;
	std::string date_YYYY_MM_DD() const;
	std::string date_YYYY_MM_DD(const std::string& separator) const;
	std::string time() const;
	std::string time_hh_mm_ss() const;
	std::string time_hh_mm_ss(const std::string& separator) const;
	std::string time_hh_mm_ss_mmmuuunnn() const;
	std::string time_mmmuuunnn(const std::string& separator) const;

	bool operator<(const TimeStamp& rhs) const;
	bool operator==(const TimeStamp& rhs) const;
	bool operator!=(const TimeStamp& rhs) const;
	bool operator<=(const TimeStamp& rhs) const;

	bool isSameDate(const TimeStamp& rhs) const;

	std::string toString() const;
	static TimeStamp fromString(const std::string& timeStamp);

    int year() const;
    int month() const;
    int day() const;
    int hour() const;
    int minute() const;
    int sec() const;
    int millis() const;
    int micros() const;
    int nanos() const;

	void add_day(int days = 1)
	{
		timeinfo.tm_mday += days;
	}

	void add_ns(int ns)
	{
		_nanos += ns;
	}

	template<class Archive>
	void serialize(Archive& archive);
	// {
	// 	archive(_millis, _micros, _nanos);//timeinfo, 
	// }

private:

	tm timeinfo;

    int _millis;
    int _micros;
    int _nanos;

	std::string getMonthName() const;
	std::string getDayName() const;

};


} //Utils
} //STI

#endif
