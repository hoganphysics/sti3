
#include "RawEventGroup.h"
#include "RawEventGroupManager.h"

#include <sti/utils/VectorMap.h>
#include <sti/utils/utils.h>

#include <vector>
#include <iostream>
#include <map>
#include <string>

using STI::Engine::RawEventGroup;
using STI::Engine::RawEventGroupManager;


int main(int argc, char **argv)
{

    // std::vector<std::string> names;
    // STI::Utils::splitString("//", "/", names);

	std::vector<RawEventGroup> groups;
	RawEventGroupManager manager(groups);

	manager.addGroup("/test");
	manager.addGroup("/test/hi");

	for (auto& m : manager.getIndexMap()) {
		std::cout << m.first << std::endl;
	}

	// RawEventGroup g("test", 0);


	return 0;
}