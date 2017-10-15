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

#ifndef MEGREZ_BASIC_H_
#define MEGREZ_BASIC_H_

#include <assert.h>
#include <stdint.h>
#include <cstddef>

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

#define MEGREZ_VERSION_MAJOR 0
#define MEGREZ_VERSION_MINOR 0
#define MEGREZ_VERSION_REVISION 6
#define MEGREZ_VERSION "0.0.6"

#define MEGREZ_STRING_EXPAND(X) #X
#define MEGREZ_STRING(X) MEGREZ_STRING_EXPAND(X)

namespace megrez {

typedef uint32_t uofs_t;
typedef int32_t sofs_t;
typedef uint16_t vofs_t;
typedef uintmax_t max_scalar_t;

template<typename T> 
struct Offset {
	uofs_t o;
	Offset() : o(0) {}
	explicit Offset(uofs_t _o) : o(_o) {}
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
	static return_type Read(const uint8_t *p, uofs_t i) {
		return EndianScalar((reinterpret_cast<const T *>(p))[i]);
	}
};

template<typename T> 
struct IndirectHelper<Offset<T>> {
	typedef const T *return_type;
	static return_type Read(const uint8_t *p, uofs_t i) {
		p += i * sizeof(uofs_t);
		return EndianScalar(reinterpret_cast<return_type>(
				p + ReadScalar<uofs_t>(p)));
	}
};

template<typename T> 
struct IndirectHelper<const T *> {
	typedef const T &return_type;
	static return_type Read(const uint8_t *p, uofs_t i) {
		return *reinterpret_cast<const T *>(p + i * sizeof(T));
	}
};

} // namespace megrez

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
#endif // defined(_MSC_VER)

#endif // MEGREZ_BASIC_H_