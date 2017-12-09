#ifndef MEGREZ_STREAM_H_
#define MEGREZ_STREAM_H_

#include "megrez/core/basic.h"

namespace megrez {
namespace io {

class MiniCopyInputStream {
 public:
	inline MiniCopyInputStream() {}
	virtual ~MiniCopyInputStream();
	virtual bool Next(const void** data, int* size) = 0;
	virtual void BackUp(int count) = 0;
	virtual bool Skip(int count) = 0;
	virtual int64 ByteCount() const = 0;
};

class MiniCopyOutputStream {
 public:
	inline MiniCopyOutputStream() {}
	virtual ~MiniCopyOutputStream();
	virtual bool Next(void** data, int* size) = 0;
	virtual void BackUp(int count) = 0;
	virtual int64 ByteCount() const = 0;
};

class OutputStream {
 private:
 	MiniCopyOutputStream* output_;
 	uint8* stream_;
 	int stream_size_;
 	int total_bytes_;
 	void Advance(int amount);
 	bool Refresh();

 public:
	OutputStream(MiniCopyOutputStream* output);
	~OutputStream();
 	bool Write(const void* data, int size);
 	bool WriteString(const std::string& str);
 	bool WriteVarint32(uint32 value);
 	bool WriteVarint64(uint64 value);
 	bool WriteTag(uint32 tag);

 	uint8* stream() { return stream_; }
 	int stream_size() { return stream_size_; }
 	int total_bytes() { return total_bytes_; }
};


} // namespace io
} // namespace megrez

#endif // MEGREZ_STREAM_H_