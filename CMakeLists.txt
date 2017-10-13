
# Copyright 2017 The Megrez Authors. All Rights Reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# 	http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)
project(Megrez)

set(CMAKE_CXX_STANDARD 11)
set(MegrezCompilerSrc
	megrez/basic.h
	megrez/builder.h
	megrez/info.h
	megrez/string.h
	megrez/struct.h
	megrez/vector.h
	megrez/util.h

	compiler/idl.h
	compiler/parser.cc
	compiler/gen_cpp.cc
	compiler/MegrezC.cc
)

include_directories(.)
add_executable(MegrezC ${MegrezCompilerSrc})