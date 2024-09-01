#include <editor/ui/editor_ui.h>

#include <core/container.h>
#include <editor/ui/log_window.h>
#include <editor/ui/main_menu_bar.h>
#include <engine/state/engine_state.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/debug/logging.h>
#include <platform/input/input.h>
#include <platform/os/imwin32.h>

#include <algorithm>
#include <format>

namespace editor {

	constexpr char LOG_WINDOW[] = "Log";
	constexpr char SCENE_WINDOW[] = "Scene";
	constexpr char PROJECT_WINDOW[] = "Project";
	constexpr char GAME_WINDOW[] = "Game";

	static void setup_docking_space(ImGuiID dockspace) {
		/* Create docks */
		ImGui::DockBuilderAddNode(dockspace); // Create a new dock node to use
		ImGui::DockBuilderSetNodeSize(dockspace, ImVec2 { 1, 1 });

		// +---------------+
		// |               |
		// |               |
		// |       1       |
		// |               |
		// |               |
		// +---------------+
		ImGuiID dock1 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.25f, nullptr, &dockspace);

		// // +---------------+
		// // |               |
		// // |       2       |   ^
		// // |               |   |
		// // +---------------+ split
		// // |       1       |
		// // +---------------+
		ImGuiID dock2 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.75f, nullptr, &dockspace);

		// //    <- split
		// // +-----+---------+
		// // |     |         |
		// // |  3  |    2    |
		// // |     |         |
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock3 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.5f, nullptr, &dockspace);

		// // +-----+---------+
		// // |  3  |         |  split
		// // +-----+    2    |    |
		// // |  4  |         |    V
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock4 = ImGui::DockBuilderSplitNode(dock3, ImGuiDir_Down, 0.5f, nullptr, &dock3);

		/* Add windows to docks */
		ImGui::DockBuilderDockWindow(LOG_WINDOW, dock1);
		ImGui::DockBuilderDockWindow(SCENE_WINDOW, dock2);
		ImGui::DockBuilderDockWindow(PROJECT_WINDOW, dock3);
		ImGui::DockBuilderDockWindow(GAME_WINDOW, dock4);
		ImGui::DockBuilderFinish(dockspace);
	}

	static engine::FontID add_font(engine::TextSystem* text_system, const char* path, uint8_t font_size) {
		return core::container::unwrap(text_system->add_ttf_font(path, font_size), [&] {
			ABORT("Failed to load font \"%s\"", path);
		});
	}

	void init_editor_ui(
		EditorUiState* ui,
		const engine::ProjectState& project,
		bool reset_docking
	) {
		init_scene_window(&ui->scene_window);

		ui->editor_fonts.system_font_id = add_font(&ui->text_system, "C:/windows/Fonts/tahoma.ttf", 13);
		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<engine::ProjectState>()(project);

		/* Setup docking */
		if (reset_docking) {
			ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			setup_docking_space(dockspace);
		}

		// add fake elements
		engine::TextID hello = ui->text_system.add_text_node(ui->editor_fonts.system_font_id, "Hello", { 0.0f, 0.0f });
		engine::TextID world = ui->text_system.add_text_node(ui->editor_fonts.system_font_id, "World", { 0.0f, 15.0f });
		ui->scene_graph.add_text_node(ui->scene_graph.root(), hello);
		ui->scene_graph.add_text_node(ui->scene_graph.root(), world);
	}

	void shutdown_editor_ui(const EditorUiState& ui) {
		shutdown_scene_window(ui.scene_window);
	}

	static std::string get_graph_node_label(const engine::GraphNode& node) {
		const char* name = "n/a";
		switch (node.type) {
			case engine::GraphNodeType::Root:
				name = "Scene";
				break;
			case engine::GraphNodeType::Text:
				name = "Text";
				break;
		}
		return std::format("{}##{}", name, node.id.value);
	}

	static void render_scene_graph_sub_tree(SceneGraphUiState* scene_graph_ui, const kpeeters::tree<engine::GraphNode>::tree_node* node_it) {
		const engine::GraphNode& node = node_it->data;

		int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (scene_graph_ui->selected_node == node.id) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (node_it->is_leaf()) {
			flags |= ImGuiTreeNodeFlags_Bullet;
		}
		if (node.type == engine::GraphNodeType::Root) {
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		std::string label = get_graph_node_label(node);
		if (scene_graph_ui->nodes[node.id].is_open) {
			ImGui::SetNextItemOpen(true);
		}
		bool node_is_open = ImGui::TreeNodeEx(label.c_str(), flags);
		scene_graph_ui->nodes[node.id].is_open = node_is_open;

		if (ImGui::IsItemClicked()) {
			scene_graph_ui->selected_node = node.id;
		}

		if (node_is_open) {
			for (auto* child = node_it->first_child; child != nullptr; child = child->next_sibling) {
				render_scene_graph_sub_tree(scene_graph_ui, child);
			}
			ImGui::TreePop();
		}
	}

	static void render_scene_graph(SceneGraphUiState* scene_graph_ui, const engine::SceneGraph& scene_graph) {
		const kpeeters::tree<engine::GraphNode>::tree_node* root_node = scene_graph.tree().begin().node;
		render_scene_graph_sub_tree(scene_graph_ui, root_node);
	}

	static void update_project_window(
		engine::ProjectState* /* project */,
		EditorUiState* ui
	) {
		/* Scene Graph */
		{
			/* Scene graph buttons */
			const auto is_selected_node = [&](const engine::GraphNode& node) { return node.id == ui->scene_graph_ui.selected_node; };
			ImGui::Text("Scene graph:");
			if (ImGui::Button("Add node")) {
				const engine::SceneGraph::Tree& tree = ui->scene_graph.tree();
				if (auto node = std::find_if(tree.begin(), tree.end(), is_selected_node); node != tree.end()) {
					auto root = ui->scene_graph.root();
					engine::TextID text_id = ui->text_system.add_text_node(ui->editor_fonts.system_font_id);
					engine::GraphNodeID child_id = ui->scene_graph.add_text_node(node, text_id);
					ui->scene_graph_ui.nodes[node->id].is_open = true;
					ui->scene_graph_ui.nodes[child_id] = UiGraphNode { .is_open = false };
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove node")) {
				const engine::SceneGraph::Tree& tree = ui->scene_graph.tree();
				if (auto node = std::find_if(tree.begin(), tree.end(), is_selected_node); node != tree.end() && node != ui->scene_graph.root()) {
					auto next_node = ui->scene_graph.remove_node(node);
					ui->scene_graph_ui.selected_node = next_node->id;
				}
			}

			/* Scene Graph Tree */
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 255, 255, 255));
				ImGui::BeginChild("SceneGraph", ImVec2(0, 0), ImGuiChildFlags_Border);

				render_scene_graph(&ui->scene_graph_ui, ui->scene_graph);

				ImGui::EndChild();
				ImGui::PopStyleColor();
			}
		}
	}

	static void update_edit_window(
		bool game_is_running,
		engine::GameState* game,
		std::vector<EditorCommand>* commands,
		bool* run_game_windowed
	) {
		if (game_is_running) {
			const int step = 1;
			ImGui::InputScalar("Game Counter", ImGuiDataType_S16, &game->counter, &step, NULL, "%d");

			if (ImGui::Button("Resume game")) {
				commands->push_back(editor::EditorCommand::RunGame);
			}
			if (ImGui::Button("Stop game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
			}
			if (ImGui::Button("Restart game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
				commands->push_back(editor::EditorCommand::RunGame);
			}
		}
		else {
			if (ImGui::Button("Run game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
				commands->push_back(editor::EditorCommand::RunGame);
			}
		}

		ImGui::Checkbox("Windowed mode", run_game_windowed);
	}

	std::vector<editor::EditorCommand> update_editor_ui(
		EditorUiState* ui,
		engine::GameState* game,
		engine::ProjectState* project,
		const platform::Input& input,
		const engine::Resources& /* resources */,
		bool unsaved_changes,
		bool game_is_running
	) {
		std::vector<editor::EditorCommand> commands;

		/* Quit */
		if (input.quit_signal_received || input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(editor::EditorCommand::Quit);
		}

		/* Dockspace */
		ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

		/* Editor Menu Bar*/
		bool reset_window_layout = false;
		if (ImWin32::BeginMainMenuBar()) {
			update_main_menu_bar(unsaved_changes, game_is_running, &commands, &reset_window_layout, &ui->show_imgui_demo);
			if (reset_window_layout) {
				setup_docking_space(dockspace);
			}
		}
		ImWin32::EndMainMenuBar();

		/* ImGui Demo */
		if (ui->show_imgui_demo) {
			ImGui::ShowDemoWindow(&ui->show_imgui_demo);
		}

		/* Log Window */
		if (ImGui::Begin(LOG_WINDOW)) {
			if (input.log) {
				update_log_window(*input.log, &commands, &ui->last_num_seen_log_entries);
			}
			else {
				ImGui::Text("Log not available!");
			}
		}
		ImGui::End();

		/* Project Window */
		if (ImGui::Begin(PROJECT_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			update_project_window(project, ui);
		}
		ImGui::End();

		/* Game Edit Window */
		if (ImGui::Begin(GAME_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			update_edit_window(game_is_running, game, &commands, &ui->run_game_windowed);
		}
		ImGui::End();

		/* Scene Window */
		ui->scene_window.is_visible = false;
		if (ImGui::Begin(SCENE_WINDOW)) {
			ui->scene_window.is_visible = true;
			update_scene_window(&ui->scene_window, &ui->scene_graph, input, &commands);
		}
		ImGui::End();

		return commands;
	}

	void render_editor_ui(
		const EditorUiState& ui,
		platform::Renderer* renderer
	) {
		render_scene_window(ui.scene_window, ui.editor_fonts, ui.text_system, renderer);
	}

} // namespace editor
