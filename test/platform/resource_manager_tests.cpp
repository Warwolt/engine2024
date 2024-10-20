#include <gtest/gtest.h>
#include <test_helper.h>

#include <platform/file/resource_manager.h>

TEST(ResourceManagerTests, InitiallyEmpty) {
	platform::ResourceManager resource_manager;

	EXPECT_TRUE(resource_manager.fonts().empty());
	EXPECT_TRUE(resource_manager.textures().empty());
}

TEST(ResourceManagerTests, LoadManifest_WithExistingFiles_AreLoadedIntoManager) {
	std::filesystem::path working_directory = std::filesystem::current_path();
	platform::ResourceManager resource_manager;
	platform::ResourceManifest manifest = {
		.fonts = { platform::FontDeclaration {
			.name = "test_font",
			.path = working_directory / "test/platform/test_data/test_font.ttf",
			.size = 16,
		} },
		.images = { platform::ImageDeclaration {
			.name = "test_image",
			.path = working_directory / "test/platform/test_data/test_image.png",
		} }
	};

	std::shared_ptr<const platform::ResourceLoadProgress> progress = resource_manager.load_manifest(manifest);
	WAIT_FOR(progress->is_done(), std::chrono::seconds(1)) {
		resource_manager.update(testing::gl_context());
	}

	ASSERT_TRUE(resource_manager.fonts().contains("test_font"));
	EXPECT_EQ(resource_manager.fonts().at("test_font").size, 16);

	ASSERT_TRUE(resource_manager.textures().contains("test_image"));
	EXPECT_EQ(resource_manager.textures().at("test_image").size.x, 16);
	EXPECT_EQ(resource_manager.textures().at("test_image").size.y, 16);
}

// TEST(ResourceManagerTests, LoadManifest_WithBadPaths_NotLoadedIntoManager) {
// }
