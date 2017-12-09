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

#ifndef MEGREZ_LITE_BUILDER_H_
#define MEGREZ_LITE_BUILDER_H_

#include "megrez/lite/vector.h"
#include "megrez/lite/string.h"
#include "megrez/core/basic.h"
#include "megrez/core/util.h"
#include "megrez/lite/offset.h"

namespace megrez {
namespace Lite {

class MegrezBuilder {
 private:
	struct FieldLoc {
		uofs_t off;
		vofs_t id;
	};
	vector_downward buf_;
	std::vector<FieldLoc> offsetbuf_;
	std::vector<uofs_t> vinfo_;
	size_t minalign_;
	bool force_defaults_;
	const char *Megrez_version_string;

 public:
	explicit MegrezBuilder(uofs_t initial_size = 1024)
		: buf_(initial_size), minalign_(1), force_defaults_(false) {
		offsetbuf_.reserve(16);
		vinfo_.reserve(16);
		EndianCheck();
		Megrez_version_string =
			"Megrez "
			MEGREZ_STRING(MEGREZ_VERSION_MAJOR) "."
			MEGREZ_STRING(MEGREZ_VERSION_MINOR) "."
			MEGREZ_STRING(MEGREZ_VERSION_REVISION);
	}

	void Clear() {
		buf_.clear();
		offsetbuf_.clear();
		vinfo_.clear();
	}

	uofs_t GetSize() const { return buf_.size(); }
	uint8_t *GetBufferPointer() const { return buf_.data(); }
	const char *GetVersionString() { return Megrez_version_string; }
	void ForceDefaults(bool fd) { force_defaults_ = fd; }
	void Pad(size_t num_bytes) { buf_.fill(num_bytes); }
	void Align(size_t elem_size) {
		if (elem_size > minalign_) minalign_ = elem_size;
		buf_.fill(PaddingBytes(buf_.size(), elem_size));
	}

	void PushBytes(const uint8_t *bytes, size_t size) { buf_.push(bytes, size); }
	void PopBytes(size_t amount) { buf_.pop(amount); }

	template<typename T> 
	void AssertScalarT() {
		static_assert(std::is_scalar<T>::value
			|| sizeof(T) == sizeof(Offset<void>),
			"T must be a scalar type");
	}

	template<typename T> 
	uofs_t PushElement(T element) {
		AssertScalarT<T>();
		T litle_endian_element = EndianScalar(element);
		Align(sizeof(T));
		PushBytes(reinterpret_cast<uint8_t *>(&litle_endian_element), sizeof(T));
		return GetSize();
	}

	template<typename T> 
	uofs_t PushElement(Offset<T> off) {
		return PushElement(ReferTo(off.o));
	}

	void TrackField(vofs_t field, uofs_t off) {
		FieldLoc fl = { off, field };
		offsetbuf_.push_back(fl);
	}

	template<typename T> 
	void AddElement(vofs_t field, T e, T def) {
		if (e == def && !force_defaults_) return;
		auto off = PushElement(e);
		TrackField(field, off);
	}

	template<typename T> 
	void AddOffset(vofs_t field, Offset<T> off) {
		if (!off.o) return;
		AddElement(field, ReferTo(off.o), static_cast<uofs_t>(0));
	}

	template<typename T> 
	void AddStruct(vofs_t field, const T *structptr) {
		if (!structptr) return;
		Align(AlignOf<T>());
		PushBytes(reinterpret_cast<const uint8_t *>(structptr), sizeof(T));
		TrackField(field, GetSize());
	}

	void AddStructOffset(vofs_t field, uofs_t off) { TrackField(field, off); }

	uofs_t ReferTo(uofs_t off) {
		Align(sizeof(uofs_t));
		assert(off <= GetSize());
		return GetSize() - off + sizeof(uofs_t);
	}

	void NotNested() { assert(!offsetbuf_.size()); }

	uofs_t StartReInfo() {
		NotNested();
		return GetSize();
	}

	uofs_t EndReInfo(uofs_t start, vofs_t numfields) {
		auto vInfoOffsetloc = PushElement<uofs_t>(0);
		buf_.fill(numfields * sizeof(vofs_t));
		auto info_object_size = vInfoOffsetloc - start;
		assert(info_object_size < 0x10000);
		PushElement<vofs_t>(info_object_size);
		PushElement<vofs_t>(FieldIndexToOffset(numfields));
		for (auto field_location = offsetbuf_.begin();
							field_location != offsetbuf_.end();
						++field_location) {
			auto pos = (vInfoOffsetloc - field_location->off);
			assert(!ReadScalar<vofs_t>(buf_.data() + field_location->id));
			WriteScalar<vofs_t>(buf_.data() + field_location->id, pos);
		}
		offsetbuf_.clear();
		auto vt1 = reinterpret_cast<vofs_t *>(buf_.data());
		auto vt1_size = *vt1;
		auto vt_use = GetSize();
		for (auto it = vinfo_.begin(); it != vinfo_.end(); ++it) {
			if (memcmp(buf_.data_at(*it), vt1, vt1_size)) continue;
			vt_use = *it;
			buf_.pop(GetSize() - vInfoOffsetloc);
			break;
		}
		if (vt_use == GetSize()) {
			vinfo_.push_back(vt_use);
		}
		WriteScalar(buf_.data_at(vInfoOffsetloc),
								static_cast<sofs_t>(vt_use) -
									static_cast<sofs_t>(vInfoOffsetloc));
		return vInfoOffsetloc;
	}

	uofs_t StartStruct(size_t alignment) {
		Align(alignment);
		return GetSize();
	}

	uofs_t EndStruct() { return GetSize(); }
	void ClearOffsets() { offsetbuf_.clear(); }
	void PreAlign(size_t len, size_t alignment) {
		buf_.fill(PaddingBytes(GetSize() + len, alignment));
	}
	template<typename T> void PreAlign(size_t len) {
		AssertScalarT<T>();
		PreAlign(len, sizeof(T));
	}

	Offset<String> CreateString(const char *str, size_t len) {
		NotNested();
		PreAlign<uofs_t>(len + 1);
		buf_.fill(1);
		PushBytes(reinterpret_cast<const uint8_t *>(str), len);
		PushElement(static_cast<uofs_t>(len));
		return Offset<String>(GetSize());
	}

	Offset<String> CreateString(const char *str) { return CreateString(str, strlen(str)); }
	Offset<String> CreateString(const std::string &str) { return CreateString(str.c_str(), str.length()); }

	uofs_t EndVector(size_t len) {
		return PushElement(static_cast<uofs_t>(len));
	}

	void StartVector(size_t len, size_t elemsize) {
		PreAlign<uofs_t>(len * elemsize);
		PreAlign(len * elemsize, elemsize);
	}

	uint8_t *ReserveElements(size_t len, size_t elemsize) {
		return buf_.make_space(len * elemsize);
	}

	template<typename T> 
	Offset<Vector<T>> CreateVector(const T *v, size_t len) {
		NotNested();
		StartVector(len, sizeof(T));
		auto i = len;
		do {
			PushElement(v[--i]);
		} while (i);
		return Offset<Vector<T>>(EndVector(len));
	}

	template<typename T> 
	Offset<Vector<T>> CreateVector(const std::vector<T> &v){
		return CreateVector(&v[0], v.size());
	}

	template<typename T> 
	Offset<Vector<const T *>> CreateVectorOfStructs(const T *v, size_t len) {
		NotNested();
		StartVector(len, AlignOf<T>());
		PushBytes(reinterpret_cast<const uint8_t *>(v), sizeof(T) * len);
		return Offset<Vector<const T *>>(EndVector(len));
	}

	template<typename T> 
	Offset<Vector<const T *>> CreateVectorOfStructs(const std::vector<T> &v) {
		return CreateVector(&v[0], v.size());
	}
	template<typename T> 
	void Finish(Offset<T> root) {
		PreAlign(sizeof(uofs_t), minalign_);
		PushElement(ReferTo(root.o));
	}
};

} // namespace Lite
} // namespace megrez

#endif // MEGREZ_LITE_BUILDER_H_