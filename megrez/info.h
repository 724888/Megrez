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

#ifndef MEGREZ_INFO_H_
#define MEGREZ_INFO_H_

#include "megrez/basic.h"

namespace megrez {

class Info {
 private:
	uint8_t data_[1];
	
 public:
	Info() {};
	Info(const Info &other) {};
	vofs_t GetOptionalFieldOffset(vofs_t field) const {
		auto vinfo = &data_ - ReadScalar<sofs_t>(&data_);
		auto vtsize = ReadScalar<vofs_t>(vinfo);
		return field < vtsize ? ReadScalar<vofs_t>(vinfo + field) : 0;
	}

	template<typename T> 
	T GetField(vofs_t field, T defaultval) const {
		auto field_offset = GetOptionalFieldOffset(field);
		return field_offset ? ReadScalar<T>(&data_[field_offset]) : defaultval;
	}

	template<typename P> 
	P GetPointer(vofs_t field) const {
		auto field_offset = GetOptionalFieldOffset(field);
		auto p = &data_[field_offset];
		return field_offset
			? reinterpret_cast<P>(p + ReadScalar<uofs_t>(p))
			: nullptr;
	}

	template<typename P> 
	P GetStruct(vofs_t field) const {
		auto field_offset = GetOptionalFieldOffset(field);
		return field_offset ? reinterpret_cast<P>(&data_[field_offset]) : nullptr;
	}

	template<typename T> 
	void SetField(vofs_t field, T val) {
		auto field_offset = GetOptionalFieldOffset(field);
		assert(field_offset);
		WriteScalar(&data_[field_offset], val);
	}

	bool CheckField(vofs_t field) const {
		return GetOptionalFieldOffset(field) != 0;
	}
};

} // namespace megrez

#endif // MEGREZ_INFO_H_