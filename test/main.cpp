#include <core/unwrap.h>
#include <core/util.h>
#include <gtest/gtest.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/graphics/gl_context.h>
#include <platform/platform_api.h>
#include <test_helper.h>

platform::OpenGLContext* g_gl_context_ptr;

namespace testing {
	platform::OpenGLContext* gl_context() {
		return g_gl_context_ptr;
	}
} // namespace testing

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	printf("DEBUG: Hello!\n");

	platform::init_test_logging();
	ASSERT(platform::initialize(), "platform::initialize failed");
	platform::Window window = core::unwrap(platform::Window::create(0, 0, SDL_WINDOW_HIDDEN, "Unit Testing"), [] {
		ABORT("platform::create_window failed");
	});
	SDL_GLContext sdl_gl_context = core::unwrap(platform::create_gl_context(window.sdl_window()), [](platform::CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", core::util::enum_to_string(error));
	});
	platform::OpenGLContext gl_context = platform::OpenGLContext(sdl_gl_context);
	g_gl_context_ptr = &gl_context;

	int error_code = RUN_ALL_TESTS();

	platform::shutdown(sdl_gl_context);
	return error_code;
}
