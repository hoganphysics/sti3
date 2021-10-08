
#include "utils.h"

//#include <stdlib.h>
//#include <ctype.h>
#include <vector>
#include <sstream>
//#include <iostream>
#include <time.h>

using std::string;

//Different compilers read numeric constants differently
#ifdef __GNUC__
#  define ULL10TO8  100000000ULL
#  define ULL10TO16 10000000000000000ULL
#else
#  define ULL10TO8  100000000
#  define ULL10TO16 10000000000000000
#endif


namespace STI
{
namespace Utils
{

std::string valueToString(const std::string& inValue)
{
	return inValue;
}

std::string printTimeFormated(double time)
{
	double baseUnit_ns = 1.0;	//units of "time" in ns

	int  s = static_cast<int>(time / (1e9 / baseUnit_ns));
	int ms = static_cast<int>((time - s * (1e9 / baseUnit_ns))/ (1e6 / baseUnit_ns));
	int us = static_cast<int>((time - s * (1e9 / baseUnit_ns) - ms * (1e6 / baseUnit_ns)) / (1e3 / baseUnit_ns));
	int ns = static_cast<int>((time - s * (1e9 / baseUnit_ns) - ms * (1e6 / baseUnit_ns) - us * (1e3 / baseUnit_ns)) / (1e0 / baseUnit_ns));

	std::stringstream timeFormatted;

	//  <Time= 23s:53ms:101us:33ns, Channel=
	//  <Time= 23 s : 53 ms : 101 us : 33 ns, Channel=
	//  <Time=23s|53ms|101us|33ns, Channel=	
	//  <Time=23s | 53ms | 101us | 33ns, Channel=
	//  <Time=23 s:53 ms:101 us:33 ns, Channel=
	//  <Time=23 s, 53 ms, 101 us, 33 ns, Channel=
	//  <Time=23s, 53ms, 101us, 33ns, Channel=

	if(s > 0) { timeFormatted << s << "s|"; }
	if(s > 0 || ms > 0) { timeFormatted << ms << "ms|"; }
	if(s > 0 || ms > 0 || us > 0) { timeFormatted << us << "us|"; }
	if(s > 0 || ms > 0 || us > 0 || ns >= 0) { timeFormatted << ns << "ns"; }

	return timeFormatted.str();
}

void convertArgs(int argc, char** argvInput, std::vector<std::string>& argvOutput)
{
	for(int i=0; i < argc; i++)
		argvOutput.push_back( std::string( argvInput[i] ) );
}

void splitString(const std::string& inString, const std::string& delimiter, std::vector<std::string>& outVector)
{
	std::string::size_type tBegin = 0;
	std::string::size_type tEnd = 0;

	// splits the sting at every delimiter
	while(tEnd != string::npos)
	{
		tBegin = inString.find_first_not_of(delimiter, tEnd);
		tEnd = inString.find_first_of(delimiter, tBegin);
		
		if(tBegin != string::npos)
			outVector.push_back(inString.substr(tBegin, tEnd - tBegin));
		else
			outVector.push_back("");
	}
}

bool isUniqueString(const std::string& value, std::vector<std::string>& list)
{
	bool found = false;

	for(unsigned i = 0; i < list.size(); i++)
	{
		found |= ( list.at(i).compare( value ) == 0 );
	}
	return !found;
}
//
//std::string generateTimeBasedFileName(const std::string& basefilename, const std::string& extension)
//{
//	time_t rawtime;
//	time(&rawtime);
//
//	return generateTimeBasedFileName(localtime(&rawtime), basefilename, extension);
//}
//
//std::string generateTimeBasedFileName(tm* timeStruct, const std::string& basefilename, const std::string& extension)
//{
//	std::stringstream fileName;
//
//	fileName << basefilename
//		<< (timeStruct->tm_mon + 1) << "_" << (timeStruct->tm_mday) << "_" << (1900 + timeStruct->tm_year)
//		<< "-"
//		<< timeStruct->tm_hour << "_" << timeStruct->tm_min << "_" << timeStruct->tm_sec;
//	
//	if (extension.compare("") != 0) {
//		fileName  << "." << extension;
//	}
//	
//	return fileName.str();
//}

std::string replaceChars(const std::string& input, const std::string& removedChars, const std::string& replacementChar)
{
	std::string result = input;
	for (unsigned i = 0; i < removedChars.length(); i++) {
		result = replaceChar(result, removedChars.substr(i, 1), replacementChar);
	}
	return result;
}

std::string replaceChar(const std::string& input, const std::string& removedChar, const std::string& replacementChar)
{
	string output = input;
	string::size_type loc;

	loc = output.find(removedChar, 0);

	while(loc != string::npos) 
	{
		output.replace(loc, 1, replacementChar);
		loc = output.find(removedChar, loc + 1);
	}

	return output;
}

std::string trim(const std::string& input, std::string white)
{
	if (input.length() == 0) {
		return input;
	}

	std::size_t start = input.find_first_not_of(white);
	std::size_t end = input.find_last_not_of(white);

	if (start == std::string::npos || end == std::string::npos) {
		return input;
	}

	return input.substr(start, end + 1);
}




}// Utils
}// STI

