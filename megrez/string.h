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

#ifndef MEGREZ_STRING_H_
#define MEGREZ_STRING_H_

#include "megrez/vector.h"

namespace megrez {

struct String : public Vector<char> {
	const char *c_str() const { 
		return reinterpret_cast<const char *>(Data()); 
	}
};

} // namespace megrez

#endif // MEGREZ_STRING_H_