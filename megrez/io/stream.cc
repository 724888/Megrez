#include "megrez/io/stream.h"
#include "megrez/core/basic.h"
#include "megrez/core/format.h"

using namespace megrez::Format;
namespace megrez {
namespace io {

OutputStream::OutputStream(MiniCopyOutputStream* output)
	: output_(output),
	  stream_(NULL),
	  stream_size_(0),
	  total_bytes_(0) {}

OutputStream::~OutputStream() {
	if (stream_size_ > 0) { output_->BackUp(stream_size_); }
}

void OutputStream::Advance(int amount) {
	stream_ += amount;
	stream_size_ -= amount;
}

bool OutputStream::Refresh() {
	void* void_stream;
	if (output_->Next(&void_stream, &stream_size_)) {
		stream_ = reinterpret_cast<uint8*>(void_stream);
		total_bytes_ += stream_size_;
		return true;
	} else {
		stream_ = NULL;
		stream_size_ = 0;
		return false;
	}
}

bool OutputStream::Write(const void* data, int size) {
	while (stream_size_ < size) {
		memcpy(stream_, data, stream_size_);
		size -= stream_size_;
		data = reinterpret_cast<const uint8*>(data) + stream_size_;
		if (!Refresh()) { return false; }
	}
	memcpy(stream_, data, size);
	Advance(size);
	return true;
}

bool OutputStream::WriteString(const std::string& str) {
	return Write(str.data(), str.size());
}

bool OutputStream::WriteVarint32(uint32 value) {
	return Write(WriteUInt32Varint(value), VarintSize32(value));
}

bool OutputStream::WriteVarint64(uint64 value) {
	return Write(WriteUInt64Varint(value), VarintSize64(value));
}

bool OutputStream::WriteTag(uint32 tag) {
	return WriteVarint32(tag);
}

} // namespace io
} // namespace megrez
