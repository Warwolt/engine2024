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
	platform::ImageDeclaration image_decl = {
		.name = "TestImage",
		.path = working_directory / "test/platform/test_data/test_image.png",
	};

	platform::ResourceManager resource_manager;
	platform::ResourceManifest manifest = {
		.images = { image_decl }
	};
	std::shared_ptr<const platform::ResourceLoadProgress> progress = resource_manager.load_manifest(manifest);

	WAIT_FOR(progress->is_done(), std::chrono::seconds(1)) {
		resource_manager.update(testing::gl_context());
	}

	EXPECT_TRUE(resource_manager.textures().contains("TestImage"));
}

// TEST(ResourceManagerTests, LoadManifest_WithBadPaths_NotLoadedIntoManager) {
// }
