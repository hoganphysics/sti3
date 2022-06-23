#ifndef STI_ENGINE_EVENTSTACKTRACE_H
#define STI_ENGINE_EVENTSTACKTRACE_H

#include <string>


namespace STI
{
namespace Engine
{


class EventStackTrace
{
public:

	std::string file() const;
	long line() const;

	std::string print(std::string indent = "       ") const;

	template<class Archive>
	void serialize(Archive& archive);

};


} //Engine
} //STI

#endif
