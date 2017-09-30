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

#ifndef MEGREZ_MEGREZ_H_
#define MEGREZ_MEGREZ_H_

#include <assert.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#if __cplusplus <= 199711L && \
		(!defined(_MSC_VER) || _MSC_VER < 1600) && \
		(!defined(__GNUC__) || \
			(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40603))
	#error A C++11 compatible compiler is required for Megrez.
	#error __cplusplus _MSC_VER __GNUC__  __GNUC_MINOR__  __GNUC_PATCHLEVEL__
#endif

#if !defined(MEGREZ_LITTLEENDIAN)
	#if defined(__GNUC__) || defined(__clang__)
		#ifdef __BIG_ENDIAN__
			#define MEGREZ_LITTLEENDIAN 0
		#else
			#define MEGREZ_LITTLEENDIAN 1
		#endif // __BIG_ENDIAN__
	#elif defined(_MSC_VER)
		#define MEGREZ_LITTLEENDIAN 1
	#else
		#error Unable to determine endianness, define MEGREZ_LITTLEENDIAN.
	#endif
#endif // !defined(MEGREZ_LITTLEENDIAN)

#define MEGREZ_VERSION_MAJOR 1
#define MEGREZ_VERSION_MINOR 0
#define MEGREZ_VERSION_REVISION 0
#define MEGREZ_STRING_EXPAND(X) #X
#define MEGREZ_STRING(X) MEGREZ_STRING_EXPAND(X)

namespace megrez {

typedef uint32_t uoffset_t;
typedef int32_t soffset_t;
typedef uint16_t voffset_t;
typedef uintmax_t largest_scalar_t;

template<typename T> 
struct Offset {
	uoffset_t o;
	Offset() : o(0) {}
	explicit Offset(uoffset_t _o) : o(_o) {}
	Offset<void> Union() const { return Offset<void>(o); }
};

inline void EndianCheck() {
	int endiantest = 1;
	assert(*reinterpret_cast<char *>(&endiantest) == MEGREZ_LITTLEENDIAN);
	(void)endiantest;
}

template<typename T> 
T EndianScalar(T t) {
	#if MEGREZ_LITTLEENDIAN
		return t;
	#else
		if (sizeof(T) == 1) {
			return t;
		} else if (sizeof(T) == 2) {
			auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&t));
			return *reinterpret_cast<T *>(&r);
		} else if (sizeof(T) == 4) {
			auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&t));
			return *reinterpret_cast<T *>(&r);
		} else if (sizeof(T) == 8) {
			auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&t));
			return *reinterpret_cast<T *>(&r);
		} else {
			assert(0);
		}
	#endif
}

template<typename T> 
T ReadScalar(const void *p) {
	return EndianScalar(*reinterpret_cast<const T *>(p));
}

template<typename T> 
void WriteScalar(void *p, T t) {
	*reinterpret_cast<T *>(p) = EndianScalar(t);
}

template<typename T> 
size_t AlignOf() {
	#ifdef _MSC_VER
		return __alignof(T);
	#else
		return alignof(T);
	#endif
}
template<typename T> 
struct IndirectHelper {
	typedef T return_type;
	static return_type Read(const uint8_t *p, uoffset_t i) {
		return EndianScalar((reinterpret_cast<const T *>(p))[i]);
	}
};
template<typename T> 
struct IndirectHelper<Offset<T>> {
	typedef const T *return_type;
	static return_type Read(const uint8_t *p, uoffset_t i) {
		p += i * sizeof(uoffset_t);
		return EndianScalar(reinterpret_cast<return_type>(
				p + ReadScalar<uoffset_t>(p)));
	}
};
template<typename T> 
struct IndirectHelper<const T *> {
	typedef const T &return_type;
	static return_type Read(const uint8_t *p, uoffset_t i) {
		return *reinterpret_cast<const T *>(p + i * sizeof(T));
	}
};
template<typename T> 
class Vector {
public:
	uoffset_t Length() const { return EndianScalar(length_); }

	typedef typename IndirectHelper<T>::return_type return_type;

	return_type Get(uoffset_t i) const {
		assert(i < Length());
		return IndirectHelper<T>::Read(Data(), i);
	}

	const void *GetStructFromOffset(size_t o) const {
		return reinterpret_cast<const void *>(Data() + o);
	}

protected:
	Vector();

	const uint8_t *Data() const {
		return reinterpret_cast<const uint8_t *>(&length_ + 1);
	}

	uoffset_t length_;
};

struct String : public Vector<char> {
	const char *c_str() const { return reinterpret_cast<const char *>(Data()); }
};
class vector_downward {
public:
	explicit vector_downward(uoffset_t initial_size)
		: reserved_(initial_size),
			buf_(new uint8_t[reserved_]),
			cur_(buf_ + reserved_) {
		assert((initial_size & (sizeof(largest_scalar_t) - 1)) == 0);
	}

	~vector_downward() { delete[] buf_; }

	void clear() { cur_ = buf_ + reserved_; }

	uoffset_t growth_policy(uoffset_t size) {
		return (size / 2) & ~(sizeof(largest_scalar_t) - 1);
	}

	uint8_t *make_space(uoffset_t len) {
		if (buf_ > cur_ - len) {
			auto old_size = size();
			reserved_ += std::max(len, growth_policy(reserved_));
			auto new_buf = new uint8_t[reserved_];
			auto new_cur = new_buf + reserved_ - old_size;
			memcpy(new_cur, cur_, old_size);
			cur_ = new_cur;
			delete[] buf_;
			buf_ = new_buf;
		}
		cur_ -= len;
		assert(size() < (1UL << (sizeof(soffset_t) * 8 - 1)) - 1);
		return cur_;
	}

	uoffset_t size() const {
		return static_cast<uoffset_t>(reserved_ - (cur_ - buf_));
	}

	uint8_t *data() const { return cur_; }
	uint8_t *data_at(uoffset_t offset) { return buf_ + reserved_ - offset; }
	void push(const uint8_t *bytes, size_t size) {
		auto dest = make_space(size);
		for (size_t i = 0; i < size; i++) dest[i] = bytes[i];
	}

	void fill(size_t zero_pad_bytes) {
		auto dest = make_space(zero_pad_bytes);
		for (size_t i = 0; i < zero_pad_bytes; i++) dest[i] = 0;
	}

	void pop(size_t bytes_to_remove) { cur_ += bytes_to_remove; }

private:
	uoffset_t reserved_;
	uint8_t *buf_;
	uint8_t *cur_;
};

inline voffset_t FieldIndexToOffset(voffset_t field_id) {
	const int fixed_fields = 2;
	return (field_id + fixed_fields) * sizeof(voffset_t);
}

inline size_t PaddingBytes(size_t buf_size, size_t scalar_size) {
	return ((~buf_size) + 1) & (scalar_size - 1);
}

class MegrezBuilder {
public:
	explicit MegrezBuilder(uoffset_t initial_size = 1024)
		: buf_(initial_size), minalign_(1), force_defaults_(false) {
		offsetbuf_.reserve(16);
		vinfos_.reserve(16);
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
		vinfos_.clear();
	}

	uoffset_t GetSize() const { return buf_.size(); }
	uint8_t *GetBufferPointer() const { return buf_.data(); }
	const char *GetVersionString() { return Megrez_version_string; }
	void ForceDefaults(bool fd) { force_defaults_ = fd; }
	void Pad(size_t num_bytes) { buf_.fill(num_bytes); }
	void Align(size_t elem_size) {
		if (elem_size > minalign_) minalign_ = elem_size;
		buf_.fill(PaddingBytes(buf_.size(), elem_size));
	}

	void PushBytes(const uint8_t *bytes, size_t size) {
		buf_.push(bytes, size);
	}

	void PopBytes(size_t amount) { buf_.pop(amount); }

	template<typename T> 
	void AssertScalarT() {
		static_assert(std::is_scalar<T>::value
			|| sizeof(T) == sizeof(Offset<void>),
			"T must be a scalar type");
	}

	template<typename T> 
	uoffset_t PushElement(T element) {
		AssertScalarT<T>();
		T litle_endian_element = EndianScalar(element);
		Align(sizeof(T));
		PushBytes(reinterpret_cast<uint8_t *>(&litle_endian_element), sizeof(T));
		return GetSize();
	}

	template<typename T> 
	uoffset_t PushElement(Offset<T> off) {
		return PushElement(ReferTo(off.o));
	}

	void TrackField(voffset_t field, uoffset_t off) {
		FieldLoc fl = { off, field };
		offsetbuf_.push_back(fl);
	}

	template<typename T> 
	void AddElement(voffset_t field, T e, T def) {
		if (e == def && !force_defaults_) return;
		auto off = PushElement(e);
		TrackField(field, off);
	}

	template<typename T> 
	void AddOffset(voffset_t field, Offset<T> off) {
		if (!off.o) return;
		AddElement(field, ReferTo(off.o), static_cast<uoffset_t>(0));
	}

	template<typename T> 
	void AddStruct(voffset_t field, const T *structptr) {
		if (!structptr) return;
		Align(AlignOf<T>());
		PushBytes(reinterpret_cast<const uint8_t *>(structptr), sizeof(T));
		TrackField(field, GetSize());
	}

	void AddStructOffset(voffset_t field, uoffset_t off) {
		TrackField(field, off);
	}

	uoffset_t ReferTo(uoffset_t off) {
		Align(sizeof(uoffset_t));
		assert(off <= GetSize());
		return GetSize() - off + sizeof(uoffset_t);
	}

	void NotNested() {
		assert(!offsetbuf_.size());
	}

	uoffset_t StartInfo() {
		NotNested();
		return GetSize();
	}

	uoffset_t EndInfo(uoffset_t start, voffset_t numfields) {
		auto vinfooffsetloc = PushElement<uoffset_t>(0);
		buf_.fill(numfields * sizeof(voffset_t));
		auto info_object_size = vinfooffsetloc - start;
		assert(info_object_size < 0x10000);
		PushElement<voffset_t>(info_object_size);
		PushElement<voffset_t>(FieldIndexToOffset(numfields));
		for (auto field_location = offsetbuf_.begin();
							field_location != offsetbuf_.end();
						++field_location) {
			auto pos = (vinfooffsetloc - field_location->off);
			assert(!ReadScalar<voffset_t>(buf_.data() + field_location->id));
			WriteScalar<voffset_t>(buf_.data() + field_location->id, pos);
		}
		offsetbuf_.clear();
		auto vt1 = reinterpret_cast<voffset_t *>(buf_.data());
		auto vt1_size = *vt1;
		auto vt_use = GetSize();
		for (auto it = vinfos_.begin(); it != vinfos_.end(); ++it) {
			if (memcmp(buf_.data_at(*it), vt1, vt1_size)) continue;
			vt_use = *it;
			buf_.pop(GetSize() - vinfooffsetloc);
			break;
		}
		if (vt_use == GetSize()) {
			vinfos_.push_back(vt_use);
		}
		WriteScalar(buf_.data_at(vinfooffsetloc),
								static_cast<soffset_t>(vt_use) -
									static_cast<soffset_t>(vinfooffsetloc));
		return vinfooffsetloc;
	}

	uoffset_t StartStruct(size_t alignment) {
		Align(alignment);
		return GetSize();
	}

	uoffset_t EndStruct() { return GetSize(); }
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
		PreAlign<uoffset_t>(len + 1);
		buf_.fill(1);
		PushBytes(reinterpret_cast<const uint8_t *>(str), len);
		PushElement(static_cast<uoffset_t>(len));
		return Offset<String>(GetSize());
	}

	Offset<String> CreateString(const char *str) {
		return CreateString(str, strlen(str));
	}

	Offset<String> CreateString(const std::string &str) {
		return CreateString(str.c_str(), str.length());
	}

	uoffset_t EndVector(size_t len) {
		return PushElement(static_cast<uoffset_t>(len));
	}

	void StartVector(size_t len, size_t elemsize) {
		PreAlign<uoffset_t>(len * elemsize);
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
		PreAlign(sizeof(uoffset_t), minalign_);
		PushElement(ReferTo(root.o));
	}

private:
	struct FieldLoc {
		uoffset_t off;
		voffset_t id;
	};
	vector_downward buf_;
	std::vector<FieldLoc> offsetbuf_;
	std::vector<uoffset_t> vinfos_;
	size_t minalign_;
	bool force_defaults_;
	const char *Megrez_version_string;
};

template<typename T> 
const T *GetRoot(const void *buf) {
	EndianCheck();
	return reinterpret_cast<const T *>(reinterpret_cast<const uint8_t *>(buf) +
		EndianScalar(*reinterpret_cast<const uoffset_t *>(buf)));
}

class Struct {
public:
	template<typename T> 
	T GetField(uoffset_t o) const {
		return ReadScalar<T>(&data_[o]);
	}

	template<typename T> 
	T GetPointer(uoffset_t o) const {
		auto p = &data_[o];
		return reinterpret_cast<T>(p + ReadScalar<uoffset_t>(p));
	}

	template<typename T> 
	T GetStruct(uoffset_t o) const {
		return reinterpret_cast<T>(&data_[o]);
	}

private:
	uint8_t data_[1];
};

class Info {
public:
	voffset_t GetOptionalFieldOffset(voffset_t field) const {
		auto vinfo = &data_ - ReadScalar<soffset_t>(&data_);
		auto vtsize = ReadScalar<voffset_t>(vinfo);
		return field < vtsize ? ReadScalar<voffset_t>(vinfo + field) : 0;
	}

	template<typename T> 
	T GetField(voffset_t field, T defaultval) const {
		auto field_offset = GetOptionalFieldOffset(field);
		return field_offset ? ReadScalar<T>(&data_[field_offset]) : defaultval;
	}

	template<typename P> 
	P GetPointer(voffset_t field) const {
		auto field_offset = GetOptionalFieldOffset(field);
		auto p = &data_[field_offset];
		return field_offset
			? reinterpret_cast<P>(p + ReadScalar<uoffset_t>(p))
			: nullptr;
	}

	template<typename P> 
	P GetStruct(voffset_t field) const {
		auto field_offset = GetOptionalFieldOffset(field);
		return field_offset ? reinterpret_cast<P>(&data_[field_offset]) : nullptr;
	}

	template<typename T> 
	void SetField(voffset_t field, T val) {
		auto field_offset = GetOptionalFieldOffset(field);
		assert(field_offset);
		WriteScalar(&data_[field_offset], val);
	}

	bool CheckField(voffset_t field) const {
		return GetOptionalFieldOffset(field) != 0;
	}
private:
	Info() {};
	Info(const Info &other) {};

	uint8_t data_[1];
};

inline size_t LookupEnum(const char **names, const char *name) {
	for (const char **p = names; *p; p++)
		if (!strcmp(*p, name))
			return p - names;
	return -1;
}

#if defined(_MSC_VER)
	#define MANUALLY_ALIGNED_STRUCT(alignment) \
		__pragma(pack(1)); \
		struct __declspec(align(alignment))
	#define STRUCT_END(name, size) \
		__pragma(pack()); \
		static_assert(sizeof(name) == size, "compiler breaks packing rules");
#elif defined(__GNUC__) || defined(__clang__)
	#define MANUALLY_ALIGNED_STRUCT(alignment) \
		_Pragma("pack(1)"); \
		struct __attribute__((aligned(alignment)))
	#define STRUCT_END(name, size) \
		_Pragma("pack()"); \
		static_assert(sizeof(name) == size, "compiler breaks packing rules");
#else
	#error Unknown compiler, please define structure alignment macros
#endif

}  // namespace megrez

#endif  // MEGREZ_MEGREZ_H_