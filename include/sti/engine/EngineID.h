#ifndef STI_ENGINE_ENGINEID_H
#define STI_ENGINE_ENGINEID_H

namespace STI
{
namespace Engine
{


class EngineID
{
public:

	EngineID();
	EngineID(int number);


	// struct EngineIDCompare 
	// {
	// 	bool operator() (const STI::Engine::EngineID& lhs, const STI::Engine::EngineID& rhs) const
	// 	{ return lhs < rhs; }
	// };

	bool operator<(const EngineID& rhs) const;
	bool operator==(const EngineID& rhs) const;
	bool operator!=(const EngineID& rhs) const;

	int getNumber() const;
	void setNumber(int number);

private:

	int engineNumber;

};


} //Engine
} //STI

#endif
