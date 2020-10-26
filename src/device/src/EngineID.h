#ifndef STI_ENGINE_ENGINEID_H
#define STI_ENGINE_ENGINEID_H

namespace STI
{
namespace Engine
{

class EngineID
{
public:

	EngineID() : engineNumber(0) {}
	EngineID(short number) : engineNumber(number) {}


	bool operator<(const EngineID& rhs) const
	{
		return(engineNumber < rhs.engineNumber);
	}
	bool operator==(const EngineID& rhs) const
	{
		return(engineNumber == rhs.engineNumber);
	}
	bool operator!=(const EngineID& rhs) const { return !((*this) == rhs); }

private:

	short engineNumber;

};


} //Engine
} //STI

#endif
