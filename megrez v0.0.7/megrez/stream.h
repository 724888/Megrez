#ifndef MEGREZ_STREAM_H_
#define MEGREZ_STREAM_H_

#include "megrez/basic.h"

namespace megrez {
namespace io {

class InputStream {
 private:
	uint8* stream_;
	int stream_size_;
	void AdvanceSize(int size);

 public:
	InputStream() : stream_size_(0) {}
	~InputStream() {}
	uint8* stream() { return stream_; }
	int stream_size() { return stream_size_; }
	void ReadTag(uint32 tag); // see format.h
	void ReadVarint32(uint32 value);
	void ReadVarint64(uint64 value);
};

class OutputStream {
 private:
	uint8* stream_;
	int stream_size_;
	void AdvanceSize(int size);

 public:
	OutputStream() : stream_size_(0) {}
	~OutputStream() {}
	uint8* stream() { return stream_; }
	int stream_size() { return stream_size_; }
	void WriteTag(uint32 tag); // see format.h
	void WriteVarint32(uint32 value);
	void WriteVarint64(uint64 value);
};


} // namespace io
} // namespace megrez

#endif // MEGREZ_STREAM_H_