
#include "NetworkConvert.h"

#include "orbTypes.h"

using STI::Network::convert;

template<>
std::string convert<::CORBA::String_member, std::string>(const ::CORBA::String_member& str)
{
	return std::string(str);
}

template<>
::CORBA::String_member convert<std::string, ::CORBA::String_member>(const std::string& str)
{
	return CORBA::string_dup(str.c_str());
}


template<>
::CORBA::UShort convert<unsigned short, ::CORBA::UShort>(const unsigned short& ushort)
{
	return static_cast<::CORBA::UShort>(ushort);
}
