#include <sti/utils/Configuration.h>
#include <sti/utils/ConfigFile.h>


#include <vector>
#include <iostream>
#include <map>
#include <string>


int main(int argc, char **argv)
{

	STI::Utils::ConfigFile config("config.ini");
	
	std::string res = config.get<std::string>("Test", "p2", "25");
	std::cout << "From file: " << res << std::endl;

	std::cout << "is list: " << (config.isList("Test", "p2") ? "True" : "False") << std::endl;

	config.addToList("Test", "p1", "[95]");
	auto vals = config.getList("Test", "p1");

	for (auto& v : vals) {
		std::cout << v << std::endl;
	}

	auto partial = config.extract("Test");


	std::cout << "From partial: " << partial.get<std::string>("Test", "p3", "failed").get() << std::endl;
	std::cout << "From partial sub: " << partial.get<std::string>("Test.STI", "parser", "failed").get() << std::endl;

	std::cout << "From subsection: " << config.get<std::string>("Test.STI", "parser", "failed").get() << std::endl;

	auto partial2 = config.extract("Test.STI");

	auto partial3 = config.extract("Numbers.STI");

	return 0;
}