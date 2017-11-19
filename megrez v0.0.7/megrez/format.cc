#include "megrez/format.h"
#include "megrez/basic.h"

namespace megrez {

// Just like protobuf 
// tag = (field_number << 3) | wire_type 
uint32 Format::MakeTag(int field_number, WireType type) {
	return MEGREZ_MAKE_TAG(field_number, type);
}

Format::WireType Format::GetTagWireType(uint32 tag) {
	return static_cast<WireType>(tag & TagTypeMask);
}

int Format::GetTagFieldNumber(uint32 tag) {
	return static_cast<int>(tag >> TagTypeBits);
}

// ZigZag Transform:  Encodes signed integers so that they can be
// effectively used with varint encoding.
//
// varint operates on unsigned integers, encoding smaller numbers into
// fewer bytes.  If you try to use it on a signed integer, it will treat
// this number as a very large unsigned integer, which means that even
// small signed numbers like -1 will take the maximum number of bytes
// (10) to encode.  ZigZagEncode() maps signed integers to unsigned
// in such a way that those with a small absolute value will have smaller
// encoded values, making them appropriate for encoding using varint.
//
// +--------------+-----------+
// |      int32 ->|     uint32|
// +--------------+-----------+
// |          0 ->|          0|
// |         -1 ->|          1|
// |          1 ->|          2|
// |         -2 ->|          3|
// |        ... ->|        ...|
// | 2147483647 ->| 4294967294|
// |-2147483648 ->| 4294967295|
// +--------------+-----------+
//
//        >> encode >>
//        << decode <<

uint32 Format::ZigZagEncode32(int32 n) { 
	return (n << 1) ^ (n >> 31); 
}
// Note:  the right-shift must be arithmetic
int32 Format::ZigZagDecode32(uint32 n) { 
	return (n >> 1) ^ - static_cast<int32>(n & 1); 
}
uint64 Format::ZigZagEncode64(int64 n) { 
	return (n << 1) ^ (n >> 63); 
}
// Note:  the right-shift must be arithmetic
int64 Format::ZigZagDecode64(uint64 n) { 
	return (n >> 1) ^ - static_cast<int64>(n & 1); 
}

uint32 Format::EncodeFloat(float value) {
	union { float f; uint32 i; };
	f = value;
	return i;
}

float Format::DecodeFloat(uint32 value) {
	union { float f; uint32 i; };
	i = value;
	return f;
}

uint64 Format::EncodeDouble(double value) {
	union { double f; uint64 i; };
	f = value;
	return i;
}

double Format::DecodeDouble(uint64 value) {
	union { double f; uint64 i; };
	i = value;
	return f;
}

// Varint Write/Read
// Only supports uint32 and uint64 since it is efficient
// For float and double, use zigzag to transform.

// E.G. 1010 1100  0000 0010
//   -> 00000010 0101100
//   -> 100101100
//   -> 256 + 32 + 8 + 4 = 300

uint8* Format::WriteUInt32Varint(uint32 value) {
	uint8* data = new uint8[5];
	int count = 0;
	do {
		data[count] = (uint8)((value & 0x7F) | 0x80);
		count++;
	} while ((value >>= 7) != 0);
	data[count-1] &= 0x7F;
	return data;
}

uint32 Format::ReadUInt32Varint(uint8* data) {
	uint32 value = data[0];
	if ((value & 0x80) == 0) { return value; }
	value &= 0x7F;

	uint32 chunk = data[1];
	value |= (chunk & 0x7F) << 7;
	if ((chunk & 0x80) == 0) { return value; }
	chunk = data[2];
	value |= (chunk & 0x7F) << 14;
	if ((chunk & 0x80) == 0) { return value; }
	chunk = data[3];
	value |= (chunk & 0x7F) << 21;
	if ((chunk & 0x80) == 0) { return value; }

	chunk = data[4];
	value |= chunk << 28;
	if ((chunk & 0xF0) == 0) { return value; }
}

uint8* Format::WriteUInt64Varint(uint64 value) {
	uint8* data = new uint8[10];
	int count = 0;
	do {
		data[count] = (uint8)((value & 0x7F) | 0x80);
		count++;
	} while ((value >>= 7) != 0);
	data[count-1] &= 0x7F;
	return data;
}

uint64 Format::ReadUInt64Varint(uint8* data) {
	uint64 value = data[0];
	if ((value & 0x80) == 0) { return value; }
	value &= 0x7F;
	
	uint64 chunk;
	for (int i=1; i<=8; i++) {
		chunk = data[i];
		value |= (chunk & 0x7F) << (i * 7);
		if ((chunk & 0x80) == 0) { return value; }
	}

	chunk = data[9];
	value |= chunk << 28;
	if ((chunk & 0xF0) == 0) { return value; }
}

size_t Format::VarintSize32(uint32 value) {
	if (value < (1 << 7)) {
		return 1;
	} else if (value < (1 << 14)) {
		return 2;
	} else if (value < (1 << 21)) {
		return 3;
	} else if (value < (1 << 28)) {
		return 4;
	} else {
		return 5;
	}
}

size_t Format::VarintSize64(uint64 value) {
	if (value < (1ull << 35)) {
		if (value < (1ull << 7)) {
			return 1;
		} else if (value < (1ull << 14)) {
			return 2;
		} else if (value < (1ull << 21)) {
			return 3;
		} else if (value < (1ull << 28)) {
			return 4;
		} else {
			return 5;
		}
	} else {
		if (value < (1ull << 42)) {
			return 6;
		} else if (value < (1ull << 49)) {
			return 7;
		} else if (value < (1ull << 56)) {
			return 8;
		} else if (value < (1ull << 63)) {
			return 9;
		} else {
			return 10;
		}
	}
}

} // namespace megrez