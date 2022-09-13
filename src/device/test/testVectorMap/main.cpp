#include <sti/utils/VectorMap.h>

#include <vector>
#include <iostream>
#include <map>
#include <string>

typedef STI::Utils::VectorMap<std::string, std::string> VM;

void print(const VM::vecType& vec)
{
	std::cout << "(";
	bool isFirst = true;
	for (auto& v : vec) {
		if (!isFirst) {
			std::cout << ", ";
		}
		std::cout << v;
		isFirst = false;
	}
	std::cout << ")" << std::endl;
}

void print(const VM::mapType& m)
{
	std::cout << "{";
	bool isFirst = true;
	for (auto& v : m) {
		if (!isFirst) {
			std::cout << ", ";
		}
		std::cout << "(" << v.first << ", " << v.second << ")";
		isFirst = false;
	}
	std::cout << "}" << std::endl;
}

int main(int argc, char **argv)
{

	STI::Utils::VectorMap<std::string, std::string> a;

	std::vector<std::string> b_vals;
	STI::Utils::VectorMap<std::string, std::string> b(b_vals);
	STI::Utils::VectorMap<std::string, std::string> c;
	// a.getVec().push_back("5");
	// a.getVec().push_back("6");
	// a.getVec().push_back("7");

	a.add("test", "val");
	a.add("test", "val2");
	a.add("test3", "val3");

	a.prepend("g","abc");

	print(a.vec());
	print(a.getIndexMap());
	// print(b_vals);

	b.add("k1", "q1");
	b.add("k2", "q2");
	b.add("k3", "q3");

	print(b_vals);
	print(b.getIndexMap());

	a.replace("test3","45");
	a.rename("test","hi");

	b.merge(a);
	print(b.getIndexMap());
	print(b_vals);

	std::string v;
	b.get("hi", v);
	unsigned x;
	b.getIndex("hi", x);
	std::cout << "test=" << v << " : " << x << std::endl;

	return 0;
}