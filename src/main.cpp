#include <GL/glew.h>

#include <engine.h>
#include <platform/assert.h>
#include <platform/commands.h>
#include <platform/file.h>
#include <platform/input/input.h>
#include <platform/input/timing.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/platform.h>
#include <platform/renderer.h>
#include <util.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <expected>
#include <optional>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using EngineLibraryHotReloader = platform::EngineLibraryHotReloader;
using LoadLibraryError = platform::LoadLibraryError;
using Primitive = platform::Primitive;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;
using CreateGLContextError = platform::CreateGLContextError;

const char* LIBRARY_NAME = "GameEngine2024";

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

	/* Read shader sources */
	const char* vertex_shader_path = "resources/shaders/shader.vert";
	const char* fragment_shader_path = "resources/shaders/shader.frag";
	std::string vertex_shader_src = util::unwrap(platform::read_file(vertex_shader_path), [&] {
		ABORT("Failed to open vertex shader \"%s\"", vertex_shader_path);
	});
	std::string fragment_shader_src = util::unwrap(platform::read_file(fragment_shader_path), [&] {
		ABORT("Failed to open fragment shader \"%s\"", fragment_shader_path);
	});

	/* Initialize OpenGL */
	Renderer renderer = Renderer(gl_context);
	ShaderProgram shader_program = util::unwrap(renderer.add_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibraryHotReloader hot_reloader = EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	engine.on_load(plog::verbose, plog::get());
	LOG_INFO("Engine library loaded");

	const char* img_path = "resources/textures/container.jpg";
	platform::Texture texture = util::unwrap(renderer.add_texture(img_path), [&](platform::AddTextureError error) {
		ABORT("Renderer::load_texture(%s) failed with: %s", img_path, util::enum_to_string(error));
	});

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input = { 0 };
	engine::State state;
	while (true) {
		/* Hot reloading */
		hot_reloader.check_hot_reloading(&engine);

		/* Input */
		platform::read_input(&input);
		input.delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();

		/* Update */
		platform::Commands commands = engine.update(&state, &input);
		if (commands.m_quit) {
			break;
		}

		/* Render */
		engine.render(&renderer, &state);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		renderer.render(window, shader_program);
	}

	platform::deinitialize(window);
	return 0;
}
