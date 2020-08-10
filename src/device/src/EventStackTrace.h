#ifndef STI_ENGINE_EVENTSTACKTRACE_H
#define STI_ENGINE_EVENTSTACKTRACE_H

#include <string>
#include <sstream>

namespace STI
{
namespace Engine
{


class EventStackTrace
{
public:

	std::string file() const { return ""; }
	long line() const { return 0; }

	std::string print(std::string indent = "       ") const
	{
		//this is a temporary implementation to mock things up
		std::stringstream trace;
		trace << indent << ">>> " << file() << ", line " << line() << "." << std::endl;

		return trace.str();
	}
};


} //Engine
} //STI

#endif
