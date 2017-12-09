cmake_minimum_required(VERSION 2.8)
project(MegrezTest)
set(MEGREZ_SOURCE_DIR ../../megrez)
set(THIRD_PARTY_DIR ../../third_party)
set(MGZ_UNITTEST_DIR ..)

include_directories(${MEGREZ_SOURCE_DIR}/../)
include_directories(${THIRD_PARTY_DIR}/googletest/include/)

set(FORMAT_UNITTEST_SRC 
	${MEGREZ_SOURCE_DIR}/core/format.h
	${MEGREZ_SOURCE_DIR}/core/format.cc
	${MGZ_UNITTEST_DIR}/format_unittest.cc )

set(STREAM_UNITTEST_SRC
	${MEGREZ_SOURCE_DIR}/core/format.h
	${MEGREZ_SOURCE_DIR}/core/format.cc
	${MEGREZ_SOURCE_DIR}/io/stream.h
	${MEGREZ_SOURCE_DIR}/io/stream.cc
	${MGZ_UNITTEST_DIR}/stream_unittest.cc )

option(FORMAT_UNITTEST "Set to ON to compile format_unittest" OFF)
option(STREAM_UNITTEST "Set to ON to compile stream_unittest" OFF)

if(FORMAT_UNITTEST)
	add_executable(format_unittest ${FORMAT_UNITTEST_SRC})
	target_link_libraries(format_unittest gtest)
endif()

if(STREAM_UNITTEST)
	add_executable(stream_unittest ${STREAM_UNITTEST_SRC})
	target_link_libraries(stream_unittest gtest)
endif()

