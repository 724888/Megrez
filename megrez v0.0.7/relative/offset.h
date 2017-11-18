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

#ifndef MEGREZ_RELATIVE_OFFSET_H_
#define MEGREZ_RELATIVE_OFFSET_H_

#include "megrez/basic.h"
#include "megrez/relative/offset.h"

namespace megrez {
namespace Relative {

typedef int8 ofs_t;
typedef uint8 uofs_t;
typedef uint16 vofs_t;
typedef uintmax_t max_scalar_t;

template<typename T>
class Offset {
 private:
 	uofs_t o_;
 public:
 	Offset() : o_(0) {}
 	explicit Offset(uofs_t _o) : o_(_o) {}
 	Offset<void> Union() const { return Offset<void>(o_); }
 	uofs_t o() { return o_; }
};

template<typename T> 
class IndirectHelper {
 public:
	typedef T return_type;
	static return_type Read(const uint8_t *p, uofs_t i) {
		return EndianScalar((reinterpret_cast<const T *>(p))[i]);
	}
};

template<typename T> 
class IndirectHelper<Offset<T>> {
 public:
	typedef const T *return_type;
	static return_type Read(const uint8_t *p, uofs_t i) {
		p += i * sizeof(uofs_t);
		return EndianScalar(reinterpret_cast<return_type>(
			p + ReadScalar<uofs_t>(p)));
	}
};

template<typename T> 
class IndirectHelper<const T *> {
 public:
	typedef const T &return_type;
	static return_type Read(const uint8_t *p, uofs_t i) {
		return *reinterpret_cast<const T *>(p + i * sizeof(T));
	}
};

} // namespace Relative
} // namespace megrez

#endif // MEGREZ_RELATIVE_OFFSET_H_