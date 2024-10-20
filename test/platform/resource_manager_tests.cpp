#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <test_helper.h>

#include <platform/file/resource_manager.h>

using namespace testing;

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
	ASSERT_TRUE(resource_manager.textures().contains("test_image"));
}

TEST(ResourceManagerTests, LoadManifest_WithInvalidPaths_NotLoadedIntoManager) {
	platform::ResourceManager resource_manager;
	platform::ResourceManifest manifest = {
		.fonts = { platform::FontDeclaration {
			.name = "test_font",
			.path = "bad_font_path.ttf",
			.size = 16,
		} },
		.images = { platform::ImageDeclaration {
			.name = "test_image",
			.path = "bad_image_path.png",
		} }
	};

	std::shared_ptr<const platform::ResourceLoadProgress> progress = resource_manager.load_manifest(manifest);
	WAIT_FOR(progress->invalid_paths.size() == 2, std::chrono::seconds(1)) {
		resource_manager.update(testing::gl_context());
	}

	EXPECT_THAT(progress->invalid_paths, UnorderedElementsAre("bad_font_path.ttf", "bad_image_path.png"));
}
