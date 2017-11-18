#include "megrez/stream.h"
#include "megrez/basic.h"
#include "megrez/format.h"

using namespace megrez::Format;
namespace megrez {
namespace io {
	
void InputStream::AdvanceSize(int size) {
	int number = stream_size_;
	stream_size_ += size;
	uint8* temp = new uint8[stream_size_];
	std::copy(stream_, stream_ + number, temp);
	delete[] stream_;
	stream_ = temp;
}
void OutputStream::AdvanceSize(int size) {
	int number = stream_size_;
	stream_size_ += size;
	uint8* temp = new uint8[stream_size_];
	std::copy(stream_, stream_ + number, temp);
	delete[] stream_;
	stream_ = temp;
}

void OutputStream::WriteVarint32(uint32 value) {
	uint8* varint_val = WriteUInt32Varint(value);
	size_t varint_len = VarintSize32(value);
	int stream_len = stream_size_;
	AdvanceSize(varint_len);
	std::copy(stream_ + stream_len + 1, stream_ + stream_len + 1 + varint_len, varint_val);
}

void OutputStream::WriteTag(uint32 tag) { WriteVarint32(tag); }

} // namespace io
} // namespace megrez
