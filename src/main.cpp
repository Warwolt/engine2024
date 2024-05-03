#include <GL/glew.h>

#include <engine.h>
#include <platform/assert.h>
#include <platform/commands.h>
#include <platform/input/input.h>
#include <platform/input/timing.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/platform.h>
#include <platform/renderer.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <magic_enum/magic_enum.h>
#include <nlohmann/json.hpp>

#include <expected>
#include <functional>
#include <optional>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;

using Primitive = platform::Primitive;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;
using CreateGLContextError = platform::CreateGLContextError;

const char* LIBRARY_NAME = "GameEngine2024";

const char* VERTEX_SHADER_SRC =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
	"    vertexColor = vec4(aColor, 1.0);\n"
	"}";

const char* FRAGMENT_SHADER_SRC =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec4 vertexColor;\n"
	"void main() {\n"
	"    FragColor = vertexColor;\n"
	"}";

namespace util {

	template <typename T, typename E, typename F>
	T unwrap(std::expected<T, E> result, F on_error) {
		if (!result.has_value()) {
			on_error(result.error());
			ABORT("util::unwrap called with std::expected not holding a value");
		}
		return result.value();
	}

	template <typename T>
	const char* enum_to_string(T value) {
		return magic_enum::enum_name(value).data();
	}

} // namespace util

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	SDL_Window* window = platform::create_window();
	ASSERT(window, "platform::create_window() returned null");

	/* Create OpenGL context */
	SDL_GLContext gl_context = util::unwrap(platform::create_gl_context(window), [](CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", util::enum_to_string(error));
	});

	/* Initialize OpenGL */
	Renderer renderer = Renderer(gl_context);
	ShaderProgram shader_program = util::unwrap(renderer.add_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC), [](ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	engine.init_logging(plog::verbose, plog::get());
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Timer hot_reload_timer;
	platform::Input input = { 0 };

	while (true) {
		/* Hot reloading */
		if (hot_reload_timer.elapsed_ms() >= 1000) {
			hot_reload_timer.reset();

			if (library_loader.library_file_has_been_modified()) {
				nlohmann::json state;

				engine.save_state(&state);
				library_loader.unload_library();

				std::expected<EngineLibrary, LoadLibraryError> load_library_result = library_loader.load_library(LIBRARY_NAME);
				if (!load_library_result.has_value()) {
					const char* error_msg = util::enum_to_string(load_library_result.error());
					ABORT("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, error_msg);
				}

				engine = load_library_result.value();
				engine.load_state(&state);
				engine.init_logging(plog::verbose, plog::get());

				LOG_INFO("Engine library reloaded");
			}
		}

		/* Input */
		platform::read_input(&input);
		input.delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();

		/* Update */
		platform::Commands commands = engine.update(&input);
		if (commands.m_quit) {
			break;
		}

		/* Render */
		engine.render(&renderer);
		renderer.render(window, shader_program);
	}

	platform::deinitialize(window);
	return 0;
}
