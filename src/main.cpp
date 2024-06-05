#include <GL/glew.h>
#include <ft2build.h>

#include <engine.h>
#include <platform/assert.h>
#include <platform/file.h>
#include <platform/font.h>
#include <platform/image.h>
#include <platform/input/input.h>
#include <platform/input/timing.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/platform.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>
#include <platform/win32.h>
#include <platform/window.h>
#include <util.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>

#include <expected>
#include <optional>

using Canvas = platform::Canvas;
using PlatformAPI = platform::PlatformAPI;
using Command = platform::Command;
using CommandType = platform::CommandType;
using CreateGLContextError = platform::CreateGLContextError;
using EngineLibrary = platform::EngineLibrary;
using EngineLibraryHotReloader = platform::EngineLibraryHotReloader;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using WindowInfo = platform::WindowInfo;

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

void init_imgui(SDL_Window* window, SDL_GLContext gl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 450");
}

void clear_screen() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void render_imgui() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void swap_buffer(SDL_Window* window) {
	SDL_GL_SwapWindow(window);
}

void deinit_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void start_imgui_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	WindowInfo window_info = util::unwrap(platform::create_window(800, 600), [] {
		ABORT("platform::create_window failed");
	});

	/* Create OpenGL context */
	SDL_GLContext gl_context = util::unwrap(platform::create_gl_context(window_info.window), [](CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", util::enum_to_string(error));
	});
	init_imgui(window_info.window, gl_context);

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
	Renderer renderer = Renderer(gl_context);
	ShaderProgram shader_program = util::unwrap(platform::add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});

	/* Load engine DLL */
	EngineLibraryLoader library_loader;
	EngineLibraryHotReloader hot_reloader = EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](LoadLibraryError error) {
		ABORT("EngineLibraryLoader::load_library(%s) failed with: %s", LIBRARY_NAME, util::enum_to_string(error));
	});
	platform::on_engine_library_loaded(&engine);
	LOG_INFO("Engine library loaded");

	/* Main loop */
	platform::Timer frame_timer;
	platform::Input input;
	platform::PlatformAPI platform;
	engine::State state;

	bool quit = false;
	Canvas canvas = platform::add_canvas(window_info.resolution.x, window_info.resolution.y);

	engine.initialize(&state);
	while (!quit) {
		/* Input */
		{
			std::vector<SDL_Event> events = platform::read_events();
			platform::process_events(&events, &input, &frame_timer, window_info.size, window_info.resolution);
			input.engine_library_is_rebuilding = hot_reloader.rebuild_command_is_running();
		}

		/* Update */
		{
			/* Hot reloading */
			hot_reloader.update(&engine);

			/* Engine update */
			start_imgui_frame();
			engine.update(&state, &input, &platform);

			/* Platform update */
			for (const Command& cmd : platform.commands()) {
				switch (cmd.type) {
					case CommandType::Quit:
						quit = true;
						break;

					case CommandType::ToggleFullscreen:
						platform::toggle_fullscreen(&window_info);
						break;

					case CommandType::ChangeResolution: {
						int width = cmd.change_resolution.width;
						int height = cmd.change_resolution.height;
						platform::change_resolution(&window_info, width, height);
						platform::free_canvas(canvas);
						canvas = platform::add_canvas(width, height);
					} break;

					case CommandType::SetWindowTitle:
						SDL_SetWindowTitle(window_info.window, cmd.set_window_title.title);
						break;

					case CommandType::RebuildEngineLibrary:
						hot_reloader.trigger_rebuild_command();
						break;
				}
			}
			platform.clear();
		}

		/* Render */
		{
			clear_screen();
			/* Render to canvas */
			{
				set_viewport(0, 0, window_info.resolution.x, window_info.resolution.y);
				set_pixel_coordinate_projection(&renderer, shader_program, window_info.resolution.x, window_info.resolution.y);
				engine.render(&renderer, &state);
				renderer.render_to_canvas(shader_program, canvas);
			}
			/* Render canvas to window */
			{
				set_viewport_to_fit_canvas(window_info.size.x, window_info.size.y, window_info.resolution.x, window_info.resolution.y);
				set_normalized_device_coordinate_projection(&renderer, shader_program);
				renderer.draw_texture(canvas.texture, platform::Rect { { -1.0f, 1.0f }, { 1.0f, -1.0f } });
				renderer.render(shader_program);
			}
			render_imgui();
			swap_buffer(window_info.window);
		}
	}

	deinit_imgui();
	engine.shutdown(&state);
	platform::free_shader_program(shader_program);
	platform::destroy_window(window_info);
	platform::shutdown(gl_context);
	return 0;
}
