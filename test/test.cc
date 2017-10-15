/* =====================================================================
Copyright 2017 The Megrez Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
========================================================================*/

#include "./test.mgz.h"
#include <iostream>
#include <chrono> 

using namespace Megrez::Test;
using namespace megrez;
using namespace std;
using namespace chrono;

const Person* Serialize() {
	vector<uint64_t> vec;
	for (size_t i=0; i<10; i++) 
		vec.push_back(i);
	MegrezBuilder mb(4096);
	auto addr = address(1, 1, 1);
	//auto name = mb.CreateString("Jiang");
	string name = "Jiang";
	auto lc = mb.CreateVector(vec);
	auto elder_ = CreatePerson(mb, &addr, 92, name, lc, Color_Black);
	mb.Finish(elder_);
	const Person *elder = GetPerson(mb.GetBufferPointer());
	return elder;
}


int main() {
	const Person* elder;
	auto start = system_clock::now();
	for (int i=1; i<=10000; i++)
		elder = Serialize();
	auto end = system_clock::now();
	auto duration = duration_cast<nanoseconds>(end - start);
	cout << "Serialization time: " 
		 << double(duration.count()) * nanoseconds::period::num / nanoseconds::period::den 
		 << "(nanoseconds).\n\n";

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