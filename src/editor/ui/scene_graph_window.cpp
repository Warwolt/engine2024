#include <editor/ui/scene_graph_window.h>

#include <engine/engine.h>

#include <imgui/imgui.h>

#include <format>

namespace editor {

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

	void SceneGraphWindow::update(
		engine::Systems* systems,
		engine::SceneGraph* scene_graph,
		engine::FontID system_font_id
	) {
		engine::TextSystem* text_system = &systems->text;

		/* Scene Graph */
		{
			/* Scene graph buttons */
			const auto is_node_selected = [&](const engine::GraphNode& node) { return m_node_is_selected[node.id]; };
			ImGui::Text("Scene graph:");
			if (ImGui::Button("Add node")) {
				const engine::SceneGraph::Tree& tree = scene_graph->tree();
				if (auto node = std::find_if(tree.begin(), tree.end(), is_node_selected); node != tree.end()) {
					auto root = scene_graph->root();
					engine::TextID text_id = text_system->add_text_node(system_font_id);
					engine::GraphNodeID child_id = scene_graph->add_text_node(node, text_id);
					m_node_is_open[node->id] = true;
					m_node_is_open[child_id] = false;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove node")) {
				const engine::SceneGraph::Tree& tree = scene_graph->tree();
				auto node = std::find_if(tree.begin(), tree.end(), is_node_selected);
				if (node != tree.end() && node != scene_graph->root()) {
					switch (node->type) {
						case engine::GraphNodeType::Root:
							break;
						case engine::GraphNodeType::Text:
							text_system->remove_text_node(scene_graph->text_id(node->id).value());
							break;
					}
					auto next_node = scene_graph->remove_node(node);
					m_node_is_selected = { { next_node->id, true } };
				}
			}

			/* Scene Graph Tree */
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 255, 255, 255));
				ImGui::BeginChild("SceneGraph", ImVec2(0, 0), ImGuiChildFlags_Border);

				_render_scene_graph(*scene_graph);

				ImGui::EndChild();
				ImGui::PopStyleColor();
			}
		}
	}

	void SceneGraphWindow::_render_scene_graph(const engine::SceneGraph& scene_graph) {
		const kpeeters::tree<engine::GraphNode>::tree_node* root_node = scene_graph.tree().begin().node;
		_render_scene_graph_sub_tree(root_node);
	}

	void SceneGraphWindow::_render_scene_graph_sub_tree(const kpeeters::tree<engine::GraphNode>::tree_node* node_it) {
		const engine::GraphNode& node = node_it->data;

		int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (m_node_is_selected[node.id]) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (node_it->is_leaf()) {
			flags |= ImGuiTreeNodeFlags_Bullet;
		}
		if (node.type == engine::GraphNodeType::Root) {
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		std::string label = get_graph_node_label(node);
		if (m_node_is_open[node.id]) {
			ImGui::SetNextItemOpen(true);
		}
		bool node_is_open = ImGui::TreeNodeEx(label.c_str(), flags);
		m_node_is_open[node.id] = node_is_open;

		if (ImGui::IsItemClicked()) {
			m_node_is_selected = { { node.id, true } };
		}

		if (node_is_open) {
			for (auto* child = node_it->first_child; child != nullptr; child = child->next_sibling) {
				_render_scene_graph_sub_tree(child);
			}
			ImGui::TreePop();
		}
	}

} // namespace editor
