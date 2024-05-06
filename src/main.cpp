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
	GLuint frame_buffer;
	GLuint canvas_texture;
	if (1) {
		// create buffer
		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

		// create texture
		glGenTextures(1, &canvas_texture);
		glBindTexture(GL_TEXTURE_2D, canvas_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// attach texture to buffer and draw buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvas_texture, 0);

		GLuint render_buffer;
		glGenRenderbuffers(1, &render_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		glRenderbufferStorage(
			GL_RENDERBUFFER,
			GL_DEPTH24_STENCIL8,
			window_width,
			window_height
		);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Couldn't setup frame buffer");

		glBindTexture(GL_TEXTURE_2D, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	}

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
		if (1) {
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		}
		// set pixel coordinate projection
		{
			glViewport(0, 0, window_width, window_height);
			float grid_offset = 0.375f; // used to avoid missing pixels
			glm::mat4 projection = glm::ortho(grid_offset, window_width + grid_offset, window_height + grid_offset, grid_offset, -1.0f, 1.0f);
			renderer.set_projection(shader_program, projection);
		}

		// render to canvas
		{
			engine.render(&renderer, &state);
			renderer.render(shader_program);
		}

		// unbind canvas
		if (1) {
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		}

		// set normalized device coordinates projection
		if (1) {
			glViewport(0, 0, window_width, window_height);
			glm::mat4 projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
			renderer.set_projection(shader_program, projection);
		}

		// render canvas texture
		if (1) {
			glUseProgram(shader_program.id);
			glBindVertexArray(shader_program.vao);
			glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);

			using Vertex = platform::Vertex;
			glm::vec4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
			float x0 = -1.0f;
			float y0 = 1.0f;
			float x1 = 1.0f;
			float y1 = -1.0f;
			Vertex quad[] = {
				// first triangle
				Vertex { .pos = { x0, y0 }, .color = white, .uv = { 0.0f, 1.0f } },
				Vertex { .pos = { x0, y1 }, .color = white, .uv = { 0.0f, 0.0f } },
				Vertex { .pos = { x1, y0 }, .color = white, .uv = { 1.0f, 1.0f } },
				// second triangle
				Vertex { .pos = { x0, y1 }, .color = white, .uv = { 0.0f, 0.0f } },
				Vertex { .pos = { x1, y0 }, .color = white, .uv = { 1.0f, 1.0f } },
				Vertex { .pos = { x1, y1 }, .color = white, .uv = { 1.0f, 0.0f } },
			};

			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), quad, GL_STATIC_DRAW);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, canvas_texture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// flip double buffer
		SDL_GL_SwapWindow(window);
	}

	platform::free_shader_program(shader_program);
	engine.deinitialize(&state);
	platform::deinitialize(window);
	return 0;
}
