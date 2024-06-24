#include <GL/glew.h>
#include <ft2build.h>

#include <engine/engine_api.h>
#include <platform/assert.h>
#include <platform/cli.h>
#include <platform/file.h>
#include <platform/font.h>
#include <platform/image.h>
#include <platform/input/input.h>
#include <platform/input/keyboard.h>
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
#include <SDL2/SDL_syswm.h>
#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>
#include <miniz/miniz.h>

#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>

const char* LIBRARY_NAME = "GameEngine2024Engine";

static void set_viewport(GLuint x, GLuint y, GLsizei width, GLsizei height) {
	glViewport(x, y, width, height);
}

static void set_viewport_to_stretch_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	int scale = (int)std::min(std::floor((float)window_width / (float)canvas_width), std::floor((float)window_height / (float)canvas_height));
	glm::ivec2 window_size = { window_width, window_height };
	glm::ivec2 scaled_canvas_size = { scale * canvas_width, scale * canvas_height };
	glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;
	glViewport(top_left.x, top_left.y, scaled_canvas_size.x, scaled_canvas_size.y);
	ASSERT(scaled_canvas_size.x <= window_width, "canvas width %d scaled larger than window width %d!", scaled_canvas_size.x, window_width);
	ASSERT(scaled_canvas_size.y <= window_height, "canvas height %d scaled larger than window height %d!", scaled_canvas_size.y, window_height);
}

static void set_viewport_to_center_canvas(int window_width, int window_height, int canvas_width, int canvas_height) {
	set_viewport((window_width - canvas_width) / 2, (window_height - canvas_height) / 2, canvas_width, canvas_height);
}

static void set_pixel_coordinate_projection(platform::Renderer* renderer, platform::ShaderProgram shader_program, int width, int height) {
	float grid_offset = 0.375f; // used to avoid missing pixels
	glm::mat4 projection = glm::ortho(grid_offset, grid_offset + width, grid_offset + height, grid_offset, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

static void set_normalized_device_coordinate_projection(platform::Renderer* renderer, platform::ShaderProgram shader_program) {
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	renderer->set_projection(shader_program, projection);
}

static void init_imgui(SDL_Window* window, SDL_GLContext gl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 450");
}

static void clear_screen() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void render_imgui() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void swap_buffer(SDL_Window* window) {
	SDL_GL_SwapWindow(window);
}

static void deinit_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

static void start_imgui_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

static HWND get_window_handle(const platform::Window* window) {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window->sdl_window(), &wmInfo);
	return wmInfo.info.win.window;
}

static std::vector<uint8_t> read_file(const std::filesystem::path& path) {
	auto length = std::filesystem::file_size(path);
	if (length == 0) {
		return {}; // empty vector
	}
	std::vector<uint8_t> buffer(length);
	std::ifstream file(path, std::ios_base::binary);
	file.read((char*)buffer.data(), length);
	file.close();
	return buffer;
}

int main(int argc, char** argv) {
	/* Parse args */
	platform::CliCommands cmd_args = util::unwrap(platform::parse_arguments(argc, argv), [](std::string error) {
		fprintf(stderr, "parse error: %s\n", error.c_str());
		printf("%s\n", platform::usage_string().c_str());
		exit(1);
	});

	if (cmd_args.print_usage) {
		printf("%s\n", platform::usage_string().c_str());
		return 0;
	}

	/* Initialize logging */
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL */
	if (!platform::initialize()) {
		ABORT("platform::initialize() failed");
	}

	/* Create window */
	const glm::ivec2 initial_window_size = { 960, 600 };
	platform::Window window = util::unwrap(platform::Window::create(initial_window_size.x, initial_window_size.y, SDL_WINDOW_RESIZABLE), [] {
		ABORT("platform::create_window failed");
	});

	/* Create OpenGL context */
	SDL_GLContext gl_context = util::unwrap(platform::create_gl_context(window.sdl_window()), [](platform::CreateGLContextError error) {
		ABORT("platform::create_gl_context() returned %s", util::enum_to_string(error));
	});
	init_imgui(window.sdl_window(), gl_context);

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
	platform::Renderer renderer = platform::Renderer(gl_context);
	platform::ShaderProgram shader_program = util::unwrap(platform::add_shader_program(vertex_shader_src.c_str(), fragment_shader_src.c_str()), [](platform::ShaderProgramError error) {
		ABORT("Renderer::add_program() returned %s", util::enum_to_string(error));
	});

	/* Load engine DLL */
	platform::EngineLibraryLoader library_loader;
	platform::EngineLibraryHotReloader hot_reloader = platform::EngineLibraryHotReloader(&library_loader, LIBRARY_NAME);
	platform::EngineLibrary engine = util::unwrap(library_loader.load_library(LIBRARY_NAME), [](platform::LoadLibraryError error) {
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
	platform::RunMode mode = cmd_args.run_game ? platform::RunMode::Game : platform::RunMode::Editor;
	platform::Canvas window_canvas = platform::add_canvas(initial_window_size.x, initial_window_size.y);
	SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

	// start game full screen
	if (mode == platform::RunMode::Game) {
		window.set_window_mode(platform::WindowMode::FullScreen);
	}

	// TEST ZIP CODE
	{
		mz_zip_archive read_archive = { 0 };

		std::filesystem::path archive_path = "D:\\dev\\cpp\\engine2024\\hello.zip";
		// load zip
		{
			bool could_read = mz_zip_reader_init_file(&read_archive, archive_path.string().c_str(), 0);
			mz_zip_error error = mz_zip_get_last_error(&read_archive);
			const char* error_str = mz_zip_get_error_string(error);
			ASSERT(could_read, "Could not open zip archive from file \"%s\": %s", archive_path.string().c_str(), error_str);
		}

		// print file names
		for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&read_archive); i++) {
			mz_zip_archive_file_stat file_stat;
			mz_zip_reader_file_stat(&read_archive, i, &file_stat);
			LOG_DEBUG("Filename: \"%s\"", file_stat.m_filename);
		}

		// print content of file `hello.txt`
		std::string hello_txt;
		{
			char* data = nullptr;
			size_t num_bytes;
			const char* file_name = "hello.txt";
			data = (char*)mz_zip_reader_extract_file_to_heap(&read_archive, file_name, &num_bytes, 0);
			LOG_DEBUG("Read %zu bytes from \"%s\"", num_bytes, file_name);
			mz_zip_error error = mz_zip_get_last_error(&read_archive);
			const char* error_str = mz_zip_get_error_string(error);
			ASSERT(data, "Could not read file \"%s\" inside archive: %s", file_name, error_str);
			hello_txt = std::string(data, num_bytes);
			mz_free((void*)data);
		}
		LOG_DEBUG("%s", hello_txt.c_str());

		// print content of file `world/world.txt`
		std::string world_txt;
		if (0) {
			char* data = nullptr;
			size_t num_bytes;
			const char* file_name = "world/world.txt";
			data = (char*)mz_zip_reader_extract_file_to_heap(&read_archive, file_name, &num_bytes, 0);
			LOG_DEBUG("Read %zu bytes from \"%s\"", num_bytes, file_name);
			mz_zip_error error = mz_zip_get_last_error(&read_archive);
			const char* error_str = mz_zip_get_error_string(error);
			ASSERT(data, "Could not read file \"%s\" inside archive: %s", file_name, error_str);
			world_txt = std::string(data, num_bytes);
			mz_free((void*)data);
		}
		LOG_DEBUG("%s", world_txt.c_str());

		// create mappin from file names to archive file indicies
		std::unordered_map<std::string, mz_uint> file_indicies;
		{
			for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&read_archive); i++) {
				mz_zip_archive_file_stat file_stat;
				mz_zip_reader_file_stat(&read_archive, i, &file_stat);
				file_indicies[file_stat.m_filename] = file_stat.m_file_index;
				LOG_DEBUG("file \"%s\" has index %zu", file_stat.m_filename, file_stat.m_file_index);
			}
		}

		// write to `hello.txt` inside temporary archive
		{
			std::filesystem::path temp_archive_path = archive_path;
			temp_archive_path.replace_filename(archive_path.filename().string() + "-temp");

			// write to string
			hello_txt.append(" :)");

			// delete temp archive if exists
			if (std::filesystem::exists(temp_archive_path)) {
				std::filesystem::remove(temp_archive_path);
			}

			// create temp archive
			mz_zip_archive write_archive = { 0 };
			{
				mz_bool result = mz_zip_writer_init_file(&write_archive, temp_archive_path.string().c_str(), 0);
				mz_zip_error error = mz_zip_get_last_error(&read_archive);
				const char* error_str = mz_zip_get_error_string(error);
				ASSERT(result, "Could not create new zip file \"%s\": %s", temp_archive_path.string().c_str(), error_str);
			}

			// write hello_txt to temp archive
			{
				bool result = mz_zip_writer_add_mem(&write_archive, "hello.txt", hello_txt.data(), hello_txt.size(), 0);
				mz_zip_error error = mz_zip_get_last_error(&read_archive);
				const char* error_str = mz_zip_get_error_string(error);
				ASSERT(result, "Could not write file \"%s\" to archive: %s", "hello.txt", error_str);
			}

			// copy non-modified file to temp archive
			{
				bool result = mz_zip_writer_add_from_zip_reader(&write_archive, &read_archive, file_indicies["world/world.txt"]);
				mz_zip_error error = mz_zip_get_last_error(&read_archive);
				const char* error_str = mz_zip_get_error_string(error);
				ASSERT(result, "Could not write file \"%s\" to archive: %s", "world/world.txt", error_str);
			}

			// finalize
			{
				bool result = mz_zip_writer_finalize_archive(&write_archive);
				ASSERT(result, "Could not finalize archive \"%s\"", temp_archive_path.string().c_str());
				mz_zip_writer_end(&write_archive);
			}

			// close reader zip
			mz_zip_reader_end(&read_archive);

			// replace old file with temp
			std::filesystem::rename(temp_archive_path, archive_path);
		}
	}

	return 0; // just do zip stuff atm

	/* Main loop */
	engine.initialize(&state);
	while (!quit) {
		/* Input */
		{
			using ButtonEvent = platform::ButtonEvent;
			constexpr size_t NUM_MOUSE_BUTTONS = 5;
			std::array<ButtonEvent, NUM_MOUSE_BUTTONS> mouse_button_events = { ButtonEvent::None };
			input.mouse.scroll_delta = 0;
			input.mouse.pos_delta = glm::vec2 { 0, 0 };

			ImGuiIO& imgui_io = ImGui::GetIO();
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL2_ProcessEvent(&event);

				switch (event.type) {
					case SDL_QUIT:
						input.quit_signal_received = true;
						break;

					case SDL_KEYDOWN:
						if (!imgui_io.WantCaptureKeyboard) {
							input.keyboard.register_event(event.key.keysym.sym, ButtonEvent::Down);
						}
						break;

					case SDL_KEYUP:
						// Note: We never let ImGui to hog up events to avoid
						// "stuck" keys when switching to an ImGui window
						input.keyboard.register_event(event.key.keysym.sym, ButtonEvent::Up);
						break;

					case SDL_MOUSEMOTION: {
						if (!imgui_io.WantCaptureMouse) {
							glm::vec2 new_mouse_pos = glm::vec2 { event.motion.x, event.motion.y };
							input.mouse.pos_delta = new_mouse_pos - input.mouse.pos;
							input.mouse.pos = new_mouse_pos;
						}
						break;
					}

					case SDL_MOUSEBUTTONDOWN:
						if (!imgui_io.WantCaptureMouse) {
							if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
								mouse_button_events[event.button.button - 1] = ButtonEvent::Down;
							}
							break;
						}

					case SDL_MOUSEBUTTONUP:
						if (!imgui_io.WantCaptureMouse) {
							if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
								mouse_button_events[event.button.button - 1] = ButtonEvent::Up;
							}
						}
						break;

					case SDL_MOUSEWHEEL:
						if (!imgui_io.WantCaptureMouse) {
							input.mouse.scroll_delta += event.wheel.y;
						}
						break;

					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
							window.on_resize(event.window.data1, event.window.data2);
						}
						break;
				}
			}

			input.keyboard.update();

			input.delta_ms = frame_timer.elapsed_ms();
			input.global_time_ms += input.delta_ms;
			frame_timer.reset();
			input.engine_is_rebuilding = hot_reloader.rebuild_command_is_running();
			input.engine_rebuild_exit_code = hot_reloader.last_exit_code();
			input.window_resolution = window_canvas.texture.size;
			input.mode = mode;
			input.mouse.left_button.update(mouse_button_events[SDL_BUTTON_LEFT - 1]);
			input.mouse.middle_button.update(mouse_button_events[SDL_BUTTON_MIDDLE - 1]);
			input.mouse.right_button.update(mouse_button_events[SDL_BUTTON_RIGHT - 1]);
			input.mouse.x1_button.update(mouse_button_events[SDL_BUTTON_X1 - 1]);
			input.mouse.x2_button.update(mouse_button_events[SDL_BUTTON_X2 - 1]);
		}

		/* Update */
		{
			/* Hot reloading */
			hot_reloader.update(&engine);

			/* Engine update */
			start_imgui_frame();
			engine.update(&state, &input, &platform);

			/* Platform update */
			for (platform::PlatformCommand& cmd : platform.commands()) {
				using PlatformCommandType = platform::PlatformCommandType;
				switch (cmd.tag()) {
					case PlatformCommandType::ChangeResolution: {
						auto& change_resolution = std::get<platform::cmd::window::ChangeResolution>(cmd);
						const int width = change_resolution.width;
						const int height = change_resolution.height;
						platform::free_canvas(window_canvas);
						window_canvas = platform::add_canvas(width, height);
					} break;

					case PlatformCommandType::Quit:
						quit = true;
						break;

					case PlatformCommandType::RebuildEngineLibrary:
						hot_reloader.trigger_rebuild_command();
						break;

					case PlatformCommandType::SetCursor: {
						auto& set_cursor = std::get<platform::cmd::cursor::SetCursor>(cmd);
						SDL_FreeCursor(cursor);
						switch (set_cursor.cursor) {
							case platform::Cursor::Arrow:
								cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
								break;

							case platform::Cursor::SizeAll:
								cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
								break;
						}
					} break;

					case PlatformCommandType::SetRunMode: {
						auto& set_run_mode = std::get<platform::cmd::app::SetRunMode>(cmd);
						mode = set_run_mode.mode;
					} break;

					case PlatformCommandType::SetWindowMode: {
						auto& set_window_mode = std::get<platform::cmd::window::SetWindowMode>(cmd);
						window.set_window_mode(set_window_mode.mode);
					} break;

					case PlatformCommandType::SetWindowTitle: {
						auto& set_window_title = std::get<platform::cmd::window::SetWindowTitle>(cmd);
						SDL_SetWindowTitle(window.sdl_window(), set_window_title.title.c_str());
					} break;

					case PlatformCommandType::ToggleFullscreen:
						window.toggle_fullscreen();
						break;

					case PlatformCommandType::LoadFileWithDialog: {
						platform::cmd::file::LoadFileWithDialog& load_file_with_dialog = std::get<platform::cmd::file::LoadFileWithDialog>(cmd);
						HWND hwnd = get_window_handle(&window);
						std::vector<uint8_t> data;
						if (std::optional<std::string> path = platform::show_load_dialog(hwnd, &load_file_with_dialog.dialog)) {
							data = read_file(std::filesystem::path { path.value() });
						}
						load_file_with_dialog.promise.set_value(std::move(data));
					} break;

					case PlatformCommandType::SaveFileWithDialog: {
						auto& save_file_with_dialog = std::get<platform::cmd::file::SaveFileWithDialog>(cmd);
						HWND hwnd = get_window_handle(&window);
						if (std::optional<std::string> path = platform::show_save_dialog(hwnd, &save_file_with_dialog.dialog)) {
							std::ofstream file;
							file.open(path.value());
							if (file.is_open()) {
								file.write((char*)save_file_with_dialog.data.data(), save_file_with_dialog.data.size());
							}
						}
					} break;
				}
			}
			platform.clear();

			/* Set Cursor */
			SDL_SetCursor(cursor);
		}

		/* Render */
		{
			clear_screen();

			/* Render to canvas */
			{
				set_viewport(0, 0, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				set_pixel_coordinate_projection(&renderer, shader_program, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				engine.render(&renderer, &state);

				renderer.set_render_canvas(window_canvas);
				renderer.render(shader_program);
				renderer.reset_render_canvas();
			}

			/* Render canvas to window */
			{
				if (window.is_fullscreen() || window.is_maximized()) {
					set_viewport_to_stretch_canvas(window.size().x, window.size().y, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				}
				else {
					set_viewport_to_center_canvas(window.size().x, window.size().y, (int)window_canvas.texture.size.x, (int)window_canvas.texture.size.y);
				}
				set_normalized_device_coordinate_projection(&renderer, shader_program);
				renderer.draw_texture(window_canvas.texture, platform::Rect { { -1.0f, 1.0f }, { 1.0f, -1.0f } });
				renderer.render(shader_program);
			}

			render_imgui();
			swap_buffer(window.sdl_window());
		}
	}

	deinit_imgui();
	engine.shutdown(&state);
	platform::free_shader_program(shader_program);
	platform::shutdown(gl_context);
	window.destroy();
	return 0;
}
