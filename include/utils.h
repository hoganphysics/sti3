#ifndef STI_UTILS_UTILS_H
#define STI_UTILS_UTILS_H

// Filename: utils.h
//

#include <string>
#include <vector>
#include <sstream>
#include <memory>

namespace STI
{
namespace Utils
{
	std::string printTimeFormated(double time);

//		std::string generateTimeBasedFileName(tm* timeStruct, const std::string& basefilename, const std::string& extension);
//		std::string generateTimeBasedFileName(const std::string& basefilename, const std::string& extension);


	void convertArgs(int argc, char** argvInput, std::vector<std::string>& argvOutput);
	void splitString(const std::string& inString, const std::string& delimiter, std::vector<std::string>& outVector);
	bool isUniqueString(const std::string& value, std::vector<std::string>& list);
	std::string replaceChar(const std::string& input, const std::string& removedChar, const std::string& replacementChar);
	std::string replaceChars(const std::string& input, const std::string& removedChars, const std::string& replacementChar);

	template<typename T> 
	bool stringToValue(const std::string& inString, T& outValue, std::ios::fmtflags numBase=std::ios::dec, std::streamsize precision=9)
	{
		//Returns true if the conversion is successful
		std::stringstream tempStream;
		tempStream.precision(precision);
		tempStream.setf( numBase, std::ios::basefield );

		tempStream << inString;
		tempStream >> outValue;

		return !tempStream.fail();
	}

	template<typename T> 
	std::string valueToString(T inValue, std::string Default="", std::ios::fmtflags numBase=std::ios::dec, std::streamsize precision=9)
	{
		std::string outString;
		std::stringstream tempStream;
		tempStream.precision(precision);
		tempStream.setf( numBase, std::ios::basefield );

		tempStream << inValue;
		outString = tempStream.str();

		if( !tempStream.fail() )
			return outString;
		else
			return Default;
	}

	template<class T>
	bool compare_unique_ptr(const std::unique_ptr<T>& a, const std::unique_ptr<T>& b)
	{
		if (a != 0 && b != 0) {
			return (*a) < (*b);
		}
		else if (b != 0) {
			return true;	//a == 0, b != 0 implies a < b
		}
		return false;		//either a != 0 and b == 0, implying a > b, or both are null
	}

} //Utils
} //STI


#endif

