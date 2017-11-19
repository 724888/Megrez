#include "megrez/format.h"
#include "gtest/gtest.h"

using namespace megrez::Format;
using namespace megrez;
using namespace testing;

/*
	uint8* varint1 = WriteUInt32Varint(4294967295);
	uint32 value1 = ReadUInt32Varint(varint1);
	std::cout << value1 << std::endl;
	uint8* varint2 = WriteUInt64Varint(uint64max);
	uint64 value2 = ReadUInt64Varint(varint2);
	std::cout << value2 << std::endl;

	std::cout << VarintSize32(4294967295) << std::endl;
	std::cout << VarintSize64(uint64max) << std::endl;

	std::cout << MakeTag(11, WIRETYPE_VARINT) << std::endl;
	std::cout << GetTagWireType(88) << std::endl;
	std::cout << GetTagFieldNumber(88) << std::endl;
*/

TEST(FormatTest, ZigZag) {
	EXPECT_EQ(19, ZigZagEncode32(-10));
	EXPECT_EQ(-10, ZigZagDecode32(19));
	EXPECT_EQ(19999, ZigZagEncode64(-10000));
	EXPECT_EQ(-10000, ZigZagDecode64(19999));
}

TEST(FormatTest, FloatDouble) {
	EXPECT_EQ(1092722098, EncodeFloat(10.101));
	EXPECT_EQ(10.101, DecodeFloat(1092722098));
	EXPECT_EQ(4621876049843660924, EncodeDouble(10.1011));
	EXPECT_EQ(10.1011, DecodeDouble(4621876049843660924));
}

int main(int argc, char **argv) {  
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();  
}
