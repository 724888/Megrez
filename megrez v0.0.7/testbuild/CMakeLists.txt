cmake_minimum_required(VERSION 2.8)
project(MegrezTest)
set(MEGREZ_SOURCE_DIR ..)
set(THIRD_PARTY_DIR ${MEGREZ_SOURCE_DIR}/third_party)
set(MGZ_UNITTEST_DIR ${MEGREZ_SOURCE_DIR}/unittest)

include_directories(${MEGREZ_SOURCE_DIR}/../)
include_directories(${THIRD_PARTY_DIR}/googletest/include/)

set(FORMAT_UNITTEST_SRC 
	${MEGREZ_SOURCE_DIR}/format.h
	${MEGREZ_SOURCE_DIR}/format.cc
	${MGZ_UNITTEST_DIR}/format_unittest.cc )
add_executable(format_unittest ${FORMAT_UNITTEST_SRC})
target_link_libraries(format_unittest gtest)