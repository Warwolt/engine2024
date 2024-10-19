#include <core/unwrap.h>
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

	platform::init_test_logging();
	platform::initialize();
	platform::Window window = platform::Window::create(0, 0, SDL_WINDOW_HIDDEN, "Unit Testing").value();
	SDL_GLContext sdl_gl_context = platform::create_gl_context(window.sdl_window()).value();
	platform::OpenGLContext gl_context = platform::OpenGLContext(sdl_gl_context);
	g_gl_context_ptr = &gl_context;

	int result = RUN_ALL_TESTS();

	platform::shutdown(sdl_gl_context);
	return result;
}
