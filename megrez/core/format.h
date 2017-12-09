#ifndef MEGREZ_FORMAT_H_
#define MEGREZ_FORMAT_H_

#include "megrez/core/basic.h"

namespace megrez {
namespace Format {

enum WireType {
	WIRETYPE_VARINT = 0,
	WIRETYPE_LENGTH_DELIMITED = 1
};

// This macro does the same thing as WireFormat::MakeTag(), but the
// result is usable as a compile-time constant, which makes it usable
// as a switch case or a template input.  WireFormat::MakeTag() is more
// type-safe, though, so prefer it if possible.
const uint32 TagTypeBits = 3;
const uint32 TagTypeMask = (1 << TagTypeBits) - 1;

#define MEGREZ_MAKE_TAG(FIELD_NUMBER, TYPE) \
	static_cast<uint32>(((FIELD_NUMBER) << TagTypeBits) | (TYPE))

uint32   MakeTag           (int field_number, WireType type);
WireType GetTagWireType    (uint32 tag);
int      GetTagFieldNumber (uint32 tag);

// To decode/encode double and float types.
// Encode a float value to uint32
uint32 EncodeFloat  (float  value);
// Decode a uint32 value to float
float  DecodeFloat  (uint32 value);
// Encode a double value to uint64
uint64 EncodeDouble (double value);
// Decode a uint64 value to double
double DecodeDouble (uint64 value);

// ZigZag see `format.cc`
uint32 ZigZagEncode32 (int32  n);
int32  ZigZagDecode32 (uint32 n);
uint64 ZigZagEncode64 (int64  n);
int64  ZigZagDecode64 (uint64 n);

// Varint writing/reading for uint32/uint64 
uint8* WriteUInt32Varint (uint32 value );
uint32 ReadUInt32Varint  (uint8* data  );
uint8* WriteUInt64Varint (uint64 value );
uint64 ReadUInt64Varint  (uint8* data  );
int VarintSize32      (uint32 value );
int VarintSize64      (uint64 value );

// Make double/float to be varint directly and easily
uint8* WriteFloatVarint  (float  value);
uint8* WriteDoubleVarint (double value);
float  ReadFloatVarint   (uint8* value);
double ReadDoubleVarint  (uint8* value);

} // namespace Format
} // namespace megrez

#endif // MEGREZ_FORMAT_H_