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

#ifndef MEGREZ_VECTOR_H_
#define MEGREZ_VECTOR_H_

#include <algorithm>
#include <assert.h>
#include <cstring>
#include "megrez/basic.h"

namespace megrez {

template<typename T> 
class Vector {
 protected:
	Vector();
	const uint8_t *Data() const {
		return reinterpret_cast<const uint8_t *>(&length_ + 1);
	}
	uofs_t length_;
	
 public:
	uofs_t Length() const { return EndianScalar(length_); }
	typedef typename IndirectHelper<T>::return_type return_type;
	return_type Get(uofs_t i) const {
		assert(i < Length());
		return IndirectHelper<T>::Read(Data(), i);
	}

	const void *GetStructFromOffset(size_t o) const {
		return reinterpret_cast<const void *>(Data() + o);
	}

};

class vector_downward {
 private:
	uofs_t reserved_;
	uint8_t *buf_;
	uint8_t *cur_;

 public:
	explicit vector_downward(uofs_t initial_size)
		: reserved_(initial_size),
			buf_(new uint8_t[reserved_]),
			cur_(buf_ + reserved_) {
		assert((initial_size & (sizeof(max_scalar_t) - 1)) == 0);
	}
	~vector_downward() { delete[] buf_; }
	void clear() { cur_ = buf_ + reserved_; }
	uofs_t growth_policy(uofs_t size) {
		return (size / 2) & ~(sizeof(max_scalar_t) - 1);
	}

	uint8_t *make_space(uofs_t len) {
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
		assert(size() < (1UL << (sizeof(sofs_t) * 8 - 1)) - 1);
		return cur_;
	}

	uofs_t size() const {
		return static_cast<uofs_t>(reserved_ - (cur_ - buf_));
	}

	uint8_t *data() const { return cur_; }
	uint8_t *data_at(uofs_t offset) { return buf_ + reserved_ - offset; }
	void push(const uint8_t *bytes, size_t size) {
		auto dest = make_space(size);
		for (size_t i = 0; i < size; i++) dest[i] = bytes[i];
	}

	void fill(size_t zero_pad_bytes) {
		auto dest = make_space(zero_pad_bytes);
		for (size_t i = 0; i < zero_pad_bytes; i++) dest[i] = 0;
	}

	void pop(size_t bytes_to_remove) { cur_ += bytes_to_remove; }
};

} // namespace megrez

#endif // MEGREZ_VECTOR_H_