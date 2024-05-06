#include <GL/glew.h>

#include <engine.h>
#include <platform/assert.h>
#include <platform/commands.h>
#include <platform/file.h>
#include <platform/image.h>
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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include <expected>
#include <optional>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using EngineLibraryHotReloader = platform::EngineLibraryHotReloader;
using LoadLibraryError = platform::LoadLibraryError;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;
using CreateGLContextError = platform::CreateGLContextError;

const char* LIBRARY_NAME = "GameEngine2024";

void on_window_resize(Renderer* renderer, ShaderProgram shader_program, float width, float height) {
	renderer->set_canvas_size(width, height);

	float grid_offset = 0.375f; // used to avoid missing pixels
	glm::mat4 projection = glm::ortho(grid_offset, width + grid_offset, height + grid_offset, grid_offset, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);

	glViewport(0, 0, (int)width, (int)height);
}

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	const int window_width = 680;
	const int window_height = 480;
	SDL_Window* window = platform::create_window(window_width, window_height);
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

	/* Initialize Renderer */
	Renderer renderer = Renderer(gl_context, window_width, window_height);
	ShaderProgram shader_program = util::unwrap(platform::add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});
	on_window_resize(&renderer, shader_program, (float)window_width, (float)window_height);

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibraryHotReloader hot_reloader = EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	engine.set_logger(plog::verbose, plog::get());
	LOG_INFO("Engine library loaded");

	// frame buffer
	GLuint framebuffer;
	GLuint canvas_texture;
	if (0) {
		// create buffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// create texture
		glGenTextures(1, &canvas_texture);
		glBindTexture(GL_TEXTURE_2D, canvas_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// attach texture to buffer and draw buffer
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, canvas_texture, 0);
		GLenum draw_buffer[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffer);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Couldn't setup frame buffer");
	}

	// quad to render texture onto

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input = { 0 };
	engine::State state;
	engine.initialize(&state);
	while (true) {
		/* Hot reloading */
		hot_reloader.check_hot_reloading(&engine);

		/* Input */
		platform::read_input(&input);
		input.delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();
		if (input.window_resized) {
			on_window_resize(&renderer, shader_program, input.window_resized->x, input.window_resized->y);
		}

		/* Update */
		platform::Commands commands = engine.update(&state, &input);
		if (commands.m_quit) {
			break;
		}

		/* Render */
		// bind canvas
		engine.render(&renderer, &state);
		renderer.render(shader_program);
		SDL_GL_SwapWindow(window);
		// unbind canvas
		// render canvas texture
	}

	platform::free_shader_program(shader_program);
	engine.deinitialize(&state);
	platform::deinitialize(window);
	return 0;
}
