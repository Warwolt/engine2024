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

using Canvas = platform::Canvas;
using CommandAPI = platform::CommandAPI;
using CommandType = platform::CommandType;
using CreateGLContextError = platform::CreateGLContextError;
using EngineLibrary = platform::EngineLibrary;
using EngineLibraryHotReloader = platform::EngineLibraryHotReloader;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;

const char* LIBRARY_NAME = "GameEngine2024";

void set_viewport(GLuint x, GLuint y, GLsizei width, GLsizei height) {
	glViewport(x, y, width, height);
}

void set_viewport_to_fit_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	int scale = (int)std::max(std::round(window_width / canvas_width), std::round(window_height / canvas_height));
	glm::ivec2 window_size = { window_width, window_height };
	glm::ivec2 scaled_canvas_size = { scale * canvas_width, scale * canvas_height };
	glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;
	glViewport(top_left.x, top_left.y, scaled_canvas_size.x, scaled_canvas_size.y);
}

void set_pixel_coordinate_projection(Renderer* renderer, ShaderProgram shader_program, int width, int height) {
	float grid_offset = 0.375f; // used to avoid missing pixels
	glm::mat4 projection = glm::ortho(grid_offset, grid_offset + width, grid_offset + height, grid_offset, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

void set_normalized_device_coordinate_projection(Renderer* renderer, ShaderProgram shader_program) {
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

void toggle_fullscreen(SDL_Window* window, bool* is_fullscreen) {
	if (*is_fullscreen) {
		SDL_SetWindowFullscreen(window, NULL);
	}
	else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	*is_fullscreen = !*is_fullscreen;
}

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	int window_width = 800;
	int window_height = 600;
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

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibraryHotReloader hot_reloader = EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	engine.set_logger(plog::verbose, plog::get());
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input;
	platform::CommandAPI commands;
	engine::State state;
	engine.initialize(&state);

	int canvas_width = window_width;
	int canvas_height = window_height;
	Canvas canvas = platform::add_canvas(canvas_width, canvas_height);

	bool is_fullscreen = false;
	bool quit = false;
	while (!quit) {
		/* Hot reloading */
		hot_reloader.check_hot_reloading(&engine);

		/* Input */
		platform::read_input(&input);
		input.delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();
		if (input.window_resized) {
			window_width = (int)input.window_resized->x;
			window_height = (int)input.window_resized->y;
		}

		/* Update */
		engine.update(&state, &input, &commands);
		for (const CommandType& command : commands.commands()) {
			switch (command) {
				case CommandType::Quit:
					quit = true;
					break;
			}
		}

		if (input.keyboard.key_pressed_now(SDLK_F11)) {
			toggle_fullscreen(window, &is_fullscreen);
		}

		/* Render to canvas */
		set_viewport(0, 0, canvas_width, canvas_height);
		set_pixel_coordinate_projection(&renderer, shader_program, canvas_width, canvas_height);
		engine.render(&renderer, &state);
		renderer.render_to_canvas(shader_program, canvas);

		/* Render canvas to window */
		set_viewport_to_fit_canvas(window_width, window_height, canvas_width, canvas_height);
		set_normalized_device_coordinate_projection(&renderer, shader_program);
		renderer.draw_texture({ -1.0f, 1.0f }, { 1.0f, -1.0f }, canvas.texture);
		renderer.render_to_window(shader_program, window);
	}

	engine.deinitialize(&state);
	platform::free_shader_program(shader_program);
	platform::deinitialize(window);
	return 0;
}
