#include <gtest/gtest.h>

#include <platform/file/resource_manager.h>

// TEST INFRA
#include <platform/graphics/window.h>
#include <platform/platform_api.h>

#include <chrono>

#define WAIT_FOR(condition, timeout)

class ResourceManagerTests : public testing::Test {
public:
};

TEST_F(ResourceManagerTests, InitiallyEmpty) {
	platform::ResourceManager resource_manager;

	EXPECT_TRUE(resource_manager.fonts().empty());
	EXPECT_TRUE(resource_manager.textures().empty());
}

TEST_F(ResourceManagerTests, LoadManifest_WithExistingFiles_AreLoadedIntoManager) {
	// SETUP PLATFORM
	// FIXME: This should maybe be done in test/main.cpp? That way we can have
	// all this stuff active and running and available for tests?
	ASSERT_TRUE(platform::initialize()) << "Platform failed to initialize";

	std::optional<platform::Window> window = platform::Window::create(0, 0, SDL_WINDOW_HIDDEN, "Test Window");
	ASSERT_TRUE(window.has_value()) << "Creating Window failed";

	std::expected<SDL_GLContext, platform::CreateGLContextError> sdl_gl_context = platform::create_gl_context(window->sdl_window());
	ASSERT_TRUE(sdl_gl_context.has_value()) << "Creating SDL_GLContext failed";
	platform::OpenGLContext gl_context = platform::OpenGLContext(sdl_gl_context.value());

	// Run test
	{
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
			resource_manager.update(&gl_context);
		}

		std::chrono::high_resolution_clock timer;
		std::chrono::steady_clock::time_point start = timer.now();
		while (!progress->is_done()) {
			resource_manager.update(&gl_context);
			if (timer.now() - start >= std::chrono::seconds(1)) {
				FAIL() << "Timeout!";
			}
		}

		EXPECT_TRUE(resource_manager.textures().contains("TestImage"));
	}

	platform::shutdown(sdl_gl_context.value());
}

// TEST_F(ResourceManagerTests, LoadManifest_WithBadPaths_NotLoadedIntoManager) {
// }
