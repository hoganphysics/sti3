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
	EngineID(int number) : engineNumber(number) {}


	// struct EngineIDCompare 
	// {
	// 	bool operator() (const STI::Engine::EngineID& lhs, const STI::Engine::EngineID& rhs) const
	// 	{ return lhs < rhs; }
	// };

	bool operator<(const EngineID& rhs) const
	{
		return(engineNumber < rhs.engineNumber);
	}
	bool operator==(const EngineID& rhs) const
	{
		return(engineNumber == rhs.engineNumber);
	}
	bool operator!=(const EngineID& rhs) const { return !((*this) == rhs); }

	int getNumber() const { return engineNumber; }
	void setNumber(int number) { engineNumber = number; }

private:

	int engineNumber;

};


} //Engine
} //STI

#endif
