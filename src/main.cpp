#include <GL/glew.h>

#include <engine.h>
#include <platform/platform.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define UNUSED(x) (void)(x)

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;

using Primitive = platform::Primitive;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;

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

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		EXIT("platform::initialize() failed");
	}

	/* Create window */
	SDL_Window* window = platform::create_window();
	ASSERT(window, "platform::create_window() returned null");

	/* Create OpenGL context */
	std::expected<SDL_GLContext, platform::CreateGLContextError> gl_context_result = platform::create_gl_context(window);
	ASSERT(gl_context_result.has_value(), "platform::create_gl_context() returned %s", create_gl_context_error_to_string(gl_context_result.error()));
	SDL_GLContext gl_context = gl_context_result.value();

	/* Initialize OpenGL */
	Renderer renderer = Renderer(gl_context);
	std::expected<ShaderProgram, ShaderProgramError> add_program_result = renderer.add_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
	ASSERT(add_program_result.has_value(), "Renderer::add_program() returned %s", platform::shader_program_error_to_string(add_program_result.error()));
	ShaderProgram shader_program = add_program_result.value();

	/* Load engine DLL */
	const char* library_name = "GameEngine2024";
	EngineLibraryLoader library_loader;
	std::expected<EngineLibrary, LoadLibraryError> load_library_result = library_loader.load_library(library_name);
	ASSERT(load_library_result.has_value(), "EngineLibraryLoader::load_library(%s) failed with: %s", library_name, load_library_error_to_string(load_library_result.error()));
	EngineLibrary engine_library = load_library_result.value();
	engine_library.on_load(plog::verbose, plog::get());
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Timer hot_reload_timer;
	engine::EngineState engine_state;
	bool quit = false;
	while (!quit) {
		const uint64_t delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();

		/* Hot reloading */
		if (hot_reload_timer.elapsed_ms() >= 1000) {
			hot_reload_timer.reset();

			if (library_loader.library_file_has_been_modified()) {
				library_loader.unload_library();
				{
					std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
					if (!load_result.has_value()) {
						const char* error_msg = load_library_error_to_string(load_result.error());
						LOG_ERROR("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s", library_name, error_msg);
					} else {
						LOG_INFO("Engine library reloaded");
						engine_library = load_result.value();
						engine_library.on_load(plog::verbose, plog::get());
					}
				}
			}
		}

		/* Input */
		{
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						quit = true;
					case SDL_KEYDOWN:
						if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
							quit = true;
						}
				}
			}
		}

		/* Update */
		engine_library.update(&engine_state, delta_ms);

		/* Render */
		engine_library.render(&renderer, &engine_state);
		renderer.render(window, shader_program);
	}

	platform::deinitialize(window);
	return 0;
}
