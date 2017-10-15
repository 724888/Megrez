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

#include "./IDLs/benchmark.mgz.h"
#include <iostream>
#include <chrono> 

using namespace benchmark;
using namespace std;
using namespace megrez;
using namespace chrono;

void serialize() {
	//MegrezBuilder mb;
	//auto id = 123456;
	//auto name = mb.CreateString("NAME");
	//auto age = 100;
	//auto gender = GENDER_TYPE_MALE;
	//auto phone_num = 10000000000;

	//auto temp = CreatePerson(mb, id, name, age, gender, phone_num);
	//mb.Finish(temp);
	//auto serialized = GetPerson(mb.GetBufferPointer());
	megrez::MegrezBuilder mb(4096);
	INFOBuilder builder(mb);

	uint8_t field1 = 1;
	int8_t field2 = 1;
	uint8_t field3 = 1;
	int16_t field4 = 1;
	uint16_t field5 = 1;
	int32_t field6 = 1;
	uint32_t field7 = 1;
	int64_t field8 = 1;
	uint64_t field9 = 1;
	float field10 = 1.0;
	double field11 = 1.0;

	builder.add_field1(field1);

	builder.add_field2(field2);
	builder.add_field3(field3);
	builder.add_field4(field4);
	builder.add_field5(field5);
	builder.add_field6(field6);
	builder.add_field7(field7);
	builder.add_field8(field8);
	builder.add_field9(field9);
	builder.add_field10(field10);
	builder.add_field11(field11);
	builder.add_field12("abcdefghijklmnopqrstuvwxyz");
	builder.add_field13(ENUM_val2);
	builder.Finish();

	auto serialized = GetINFO(builder.mb_.GetBufferPointer());
}

int main() {
	cout << "Megrez Benchmark" << endl;
	auto start = system_clock::now();
	for (int i=1; i<=10000; i++)
		serialize();

	auto end = system_clock::now();
	auto duration = duration_cast<nanoseconds>(end - start);
	cout << "Serialization time: " 
		 << double(duration.count()) * nanoseconds::period::num / nanoseconds::period::den 
		 << "(nanoseconds).\n";

	INFO info;
	cout << info.field12()->c_str();
	cin.get();
	cin.get();
	return 0;
}