#include <engine/editor.h>

#include <engine/game_state.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/platform_api.h>
#include <util.h>

#include <imgui/imgui.h>
#include <nlohmann/json.hpp>

namespace engine {

	void update_editor(
		EditorState* editor,
		GameState* game,
		const platform::Input* input,
		platform::PlatformAPI* platform
	) {
		const bool editor_is_running = input->mode == platform::RunMode::Editor;

		/* Editor Menu Bar*/
		if (editor_is_running && ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					LOG_DEBUG("New Project");
					*game = {};
				}
				if (ImGui::MenuItem("Load Project")) {
					LOG_DEBUG("Load Project");
					platform::FileExplorerDialog dialog = {
						.title = "Load project",
						.description = "JSON (*.json)",
						.extension = "json",
					};
					editor->load_project_future = platform->load_file_with_dialog(dialog);
				}
				if (ImGui::MenuItem("Save Project")) {
					LOG_DEBUG("Save Project");
					nlohmann::json json_object = {
						{ "counter", game->counter }
					};
					std::string data = json_object.dump();
					platform::FileExplorerDialog dialog = {
						.title = "Save project",
						.description = "JSON (*.json)",
						.extension = "json",
					};
					platform->save_file_with_dialog(std::vector<uint8_t>(data.begin(), data.end()), dialog);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (util::future_is_ready(editor->load_project_future)) {
			std::vector<uint8_t> buffer = editor->load_project_future.get();
			if (!buffer.empty()) {
				nlohmann::json json_object = nlohmann::json::parse(buffer);
				game->counter = json_object["counter"];
			}
		}

		/* Editor Window */
		if (editor_is_running && ImGui::Begin("Editor Window")) {
			const int step = 1;
			ImGui::InputScalar("Counter", ImGuiDataType_S16, &game->counter, &step, NULL, "%d");
			if (ImGui::Button("Run game")) {
				platform->set_run_mode(platform::RunMode::Game);
				game->counter = 0;
				game->time_ms = 0;
			}
			if (ImGui::Button("Resume game")) {
				platform->set_run_mode(platform::RunMode::Game);
			}

			ImGui::End();
		}
	}

} // namespace engine
