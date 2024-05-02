#include <gtest/gtest.h>
#include <platform/logging.h>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	platform::init_test_logging();
	return RUN_ALL_TESTS();
}
