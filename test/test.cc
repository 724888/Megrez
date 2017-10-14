
#include "./test.mgz.h"
#include <iostream>

using namespace Megrez::Test;
using namespace megrez;
using namespace std;

const Person* Serialize() {
	vector<uint64_t> vec;
	for (size_t i=0; i<10; i++) 
		vec.push_back(i);
	MegrezBuilder mb;
	auto addr = address(1, 1, 1);
	auto name = mb.CreateString("Jiang");
	auto lc = mb.CreateVector(vec);
	auto elder_ = CreatePerson(mb, &addr, 92, name, lc, Color_Black);
	mb.Finish(elder_);
	const Person *elder = GetPerson(mb.GetBufferPointer());
	return elder;
}


int main() {
	const Person* elder = Serialize();
	cout << elder->name()->c_str() << endl << endl;
	cout << elder->age() << endl << endl;
	cout << elder->Address()->block() << endl;
	cout << elder->Address()->street() << endl;
	cout << elder->Address()->number() << endl << endl;
	cout << elder->LifeContinue()->Get(1) << endl;
	cout << elder->LifeContinue()->Get(2) << endl;
	cout << elder->LifeContinue()->Get(3) << endl << endl;
	if (elder->GlassColor() == Color_Black)
		cout << "Black Glass [=]-[=]!";
	cin.get();
	cin.get();
	return 0;
}