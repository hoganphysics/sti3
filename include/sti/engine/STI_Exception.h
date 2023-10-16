#ifndef STI_ENGINE_STI_EXCEPTION_H
#define STI_ENGINE_STI_EXCEPTION_H

#include <string>
#include <exception>


namespace STI
{
namespace Engine
{


class STI_Exception : public std::exception
{
public:

	STI_Exception(const std::string& message) : _message(message) {};
	virtual ~STI_Exception() throw() { };

	inline std::string printMessage() const { return _message; };

private:

	std::string _message;
};

} //Engine
} //STI


#endif
