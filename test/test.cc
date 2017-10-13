
#include "./test.mgz.h"
#include <iostream>

using namespace Megrez::Test;
using namespace megrez;
using namespace std;

int main() {
	vector<uint64_t> vec;
	for (size_t i=0; i<10; i++) 
		vec.push_back(i);
	MegrezBuilder mb;
	auto addr = address(1, 1, 1);
	auto name = mb.CreateString("Jiang");
	auto lc = mb.CreateVector(vec);
	auto elder_ = CreatePerson(mb, &addr, 92, name, lc, Color_Black);
	mb.Finish(elder_);
	auto elder = GetPerson(mb.GetBufferPointer());
	cout << elder -> name() -> c_str() << endl
		 << elder -> age();

	cin.get();
	cin.get();
	return 0;
}