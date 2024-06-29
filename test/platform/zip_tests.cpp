#include <gtest/gtest.h>

#include <platform/zip.h>

#include <filesystem>

class ZipTests : public testing::Test {
public:
	static std::filesystem::path m_test_zip_path;
	static void SetUpTestSuite() {
		const std::filesystem::path current_path = std::filesystem::current_path();
		m_test_zip_path = current_path / "test/platform/zip_tests.zip";
		ASSERT_TRUE(std::filesystem::is_regular_file(m_test_zip_path)) << "The test zip archive is missing, cannot run zip tests.";
	}
};

std::filesystem::path ZipTests::m_test_zip_path;

TEST_F(ZipTests, Hello) {
	// TODO write some zip archive unit tests
	EXPECT_EQ(1 + 1, 3) << "Hello world!";
}
