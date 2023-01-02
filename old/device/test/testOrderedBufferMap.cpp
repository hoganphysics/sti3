
#include "utils/OrderedBufferMap.h"
#include <sti/utils/utils.h>

#include <iostream>
using std::cout;
using std::endl;

class A
{
public:
	A(unsigned val) : val(val) {}
	unsigned val;
};

int main2(int argc, char **argv)
{
	STI::Utils::OrderedBufferMap<unsigned, A> buffer(4);

	for (unsigned i = 0; i < 9; ++i) {
		A a(10 * i);
		buffer.add(i, a);
	}

	A tmp(0);
	for (unsigned j = 0; j < 9; ++j) {
		cout << "A(" << (buffer.get(j, tmp) ? STI::Utils::valueToString(tmp.val) : "missing") << ")" << endl;
	}

	return 0;
}