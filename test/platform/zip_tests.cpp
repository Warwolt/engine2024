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

TEST_F(ZipTests, DefaultConstructedArchive_IsNotYetValid) {
	platform::FileArchive archive;
	EXPECT_FALSE(archive.is_valid());
}

TEST_F(ZipTests, ArchiveLoadedFromZipFile_IfZipFileExists_ArchiveIsValid) {
	platform::FileArchive archive;
	std::optional<std::string> error = platform::FileArchive::open_from_file(&archive, m_test_zip_path.c_str());
	EXPECT_FALSE(error.has_value()) << "Opening an exising archive should not result in any errors";
	EXPECT_TRUE(archive.is_valid());
}
