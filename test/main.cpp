#include <gtest/gtest.h>
#include <platform/debug/logging.h>
#include <platform/graphics/font.h>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	platform::init_test_logging();
	platform::initialize_fonts();
	return RUN_ALL_TESTS();
}
