#ifndef MEGREZ_BASIC_H_
#define MEGREZ_BASIC_H_

#include <assert.h>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <memory>
#include <iostream>
#include <sstream>

typedef unsigned int uint;

#ifdef _MSC_VER
	typedef __int8  int8;
	typedef __int16 int16;
	typedef __int32 int32;
	typedef __int64 int64;

	typedef unsigned __int8  uint8;
	typedef unsigned __int16 uint16;
	typedef unsigned __int32 uint32;
	typedef unsigned __int64 uint64;
#else
	typedef int8_t  int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;

	typedef uint8_t  uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
#endif // _MSC_VER

const int32 int32max = 0x7FFFFFFF;
const int32 int32min = -int32max - 1;
const int64 int64max = 0x7FFFFFFFFFFFFFFF;
const int64 int64min = -int64max - 1;
const uint32 uint32max = 0xFFFFFFFFu;
const uint64 uint64max = 0xFFFFFFFFFFFFFFFF;

#define MEGREZ_VERSION_MAJOR 0
#define MEGREZ_VERSION_MINOR 0
#define MEGREZ_VERSION_REVISION 7
#define MEGREZ_VERSION_SUFFIX ""
#define MEGREZ_VERSION 0000007

#if __cplusplus <= 199711L && \
		(!defined(_MSC_VER) || _MSC_VER < 1600) && \
		(!defined(__GNUC__) || \
			(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40603))
	#error A C++11 compatible compiler is required for Megrez
	#error Check the versions below:
	#error __cplusplus _MSC_VER __GNUC__  __GNUC_MINOR__  __GNUC_PATCHLEVEL__
#endif

#if (!defined(_MSC_VER) || _MSC_VER >= 1900) && \
		(!defined(__GNUC__) || (__GNUC__ * 100 + __GNUC_MINOR__ >= 406))
	#define MEGREZ_CONSTEXPR constexpr
#else
	#define MEGREZ_CONSTEXPR
#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ * 10 + __GNUC_MINOR__ >= 46 || \
		defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
	#define MEGREZ_NOEXCEPT noexcept
#else
	#define MEGREZ_NOEXCEPT
#endif

#if !defined(__clang__) && \
	 defined(__GNUC__) && \
	 (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40600)
	// Backwards compatability for g++ 4.4, and 4.5 which don't have the nullptr
	// and constexpr keywords. Note the __clang__ check is needed, because clang
	// presents itself as an older GNUC compiler.
	#ifndef nullptr_t
		const class nullptr_t {
		 public:
			template<class T> inline operator T*() const { return 0; }
		 private:
			void operator&() const;
		} nullptr = {};
	#endif // nullptr_t
	#ifndef constexpr
		#define constexpr const
	#endif // constexpr
#endif

// Megrez use littleendian to store data since it is fast.
#if defined(__s390x__)
	#define MEGREZ_LITTLE_ENDIAN 0
#endif // __s390x__
#if !defined(MEGREZ_LITTLE_ENDIAN)
	#if defined(__GNUC__) || defined(__clang__)
		#ifdef __BIG_ENDIAN__
			#define MEGREZ_LITTLE_ENDIAN 0
		#else
			#define MEGREZ_LITTLE_ENDIAN 1
		#endif // __BIG_ENDIAN__
	#elif defined(_MSC_VER)
		#if defined(_M_PPC)
			#define MEGREZ_LITTLE_ENDIAN 0
		#else
			#define MEGREZ_LITTLE_ENDIAN 1
		#endif
	#else
		#error Cannot determine endianness.
	#endif
#endif // !defined(MEGREZ_LITTLE_ENDIAN)

#define MEGREZ_ENDIAN MEGREZ_LITTLE_ENDIAN
// Shortcut, 0 stands for big endian, 1 stands for little endian.
inline bool IsLittleEndian() { return MEGREZ_ENDIAN; }

namespace megrez {

inline void EndianCheck() {
	int endiantest = 1;
	assert(*reinterpret_cast<char *>(&endiantest) == MEGREZ_ENDIAN);
	(void)endiantest;
}

template<typename T>
T EndianSwap(T t) {
	#if defined(_MSC_VER)
		#define MEGREZ_BYTESWAP16 _byteswap_ushort
		#define MEGREZ_BYTESWAP32 _byteswap_ulong
		#define MEGREZ_BYTESWAP64 _byteswap_uint64
	#else
		#if defined(__GNUC__) && __GNUC__ * 100 + __GNUC_MINOR__ < 408 && !defined(__clang__)
			// __builtin_bswap16 was missing prior to GCC 4.8.
			#define MEGREZ_BYTESWAP16(x) \
				static_cast<uint16_t>(__builtin_bswap32(static_cast<uint32_t>(x) << 16))
		#else
			#define MEGREZ_BYTESWAP16 __builtin_bswap16
		#endif
		#define MEGREZ_BYTESWAP32 __builtin_bswap32
		#define MEGREZ_BYTESWAP64 __builtin_bswap64
	#endif
	if (sizeof(T) == 1) { 
		// Compile-time if-then's.
		return t;
	} else if (sizeof(T) == 2) {
		auto r = MEGREZ_BYTESWAP16(*reinterpret_cast<uint16_t *>(&t));
		return *reinterpret_cast<T *>(&r);
	} else if (sizeof(T) == 4) {
		auto r = MEGREZ_BYTESWAP32(*reinterpret_cast<uint32_t *>(&t));
		return *reinterpret_cast<T *>(&r);
	} else if (sizeof(T) == 8) {
		auto r = MEGREZ_BYTESWAP64(*reinterpret_cast<uint64_t *>(&t));
		return *reinterpret_cast<T *>(&r);
	} else {
		assert(0);
	}
}

template<typename T> 
T EndianScalar(T t) {
	#if MEGREZ_ENDIAN
		return t;
	#else
		return EndianSwap<T>(t);
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

} // namespace megrez

#endif // MEGREZ_BASIC_H_