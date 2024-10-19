#include <gtest/gtest.h>

#include <platform/file/resource_manager.h>

class ResourceManagerTests : public testing::Test {
public:
	static void SetUpTestSuite() {
		// m_work_directory = std::filesystem::current_path();
		// m_test_archive_path = m_work_directory / "test/platform/zip_tests.zip";
		// m_write_archive_path = m_work_directory / "write_test.zip";
		// ASSERT_TRUE(std::filesystem::is_regular_file(m_test_archive_path)) << "The test zip archive is missing, cannot run zip tests!";
	}
};

TEST_F(ResourceManagerTests, InitiallyEmpty) {
	platform::ResourceManager resource_manager;

	EXPECT_TRUE(resource_manager.fonts().empty());
	EXPECT_TRUE(resource_manager.textures().empty());
}
