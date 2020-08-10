#ifndef STI_ENGINE_ENGINEID_H
#define STI_ENGINE_ENGINEID_H

namespace STI
{
namespace Engine
{

class EngineID
{
public:

	EngineID() {}
	EngineID& operator= (const EngineID& rhs)
	{
		engineNumber = rhs.engineNumber;
		parseTime = rhs.parseTime;
		return (*this);
	}

	bool exactMatch(const EngineID& rhs) const
	{
		return (*this) == rhs && parseTime == rhs.parseTime;
	}

	short engineNumber;
	long parseTime;

	//The comparison operators ignore parseTime.  This way the collector only keeps one copy of each event engine.
	//Before reusing an engine, the manager must check the parseTime independently.

	bool operator<(const EngineID& rhs) const
	{
		return(engineNumber < rhs.engineNumber);
	}
	bool operator==(const EngineID& rhs) const
	{
		return(engineNumber == rhs.engineNumber);
	}
	bool operator!=(const EngineID& rhs) const { return !((*this) == rhs); }

};


} //Engine
} //STI

#endif
