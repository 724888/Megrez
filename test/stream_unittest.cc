#include "megrez/io/stream.h"
#include "megrez/core/format.h"
#include "gtest/gtest.h"

using namespace megrez::io;
using namespace megrez::Format;
using namespace megrez;
using namespace testing;

TEST(StreamTest, Output) {
	MiniCopyOutputStream* mcos = NULL;
	OutputStream os(mcos);
	bool a1 = os.WriteTag(81);
	bool a2 = os.WriteVarint32(100);
	bool a3 = os.WriteVarint64(1000);
	EXPECT_TRUE(a1);
	EXPECT_TRUE(a2);
	EXPECT_TRUE(a3);
}

int main(int argc, char **argv) {
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}