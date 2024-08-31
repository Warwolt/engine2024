#include <editor/ui/log_window.h>

#include <imgui/imgui.h>

#include <platform/debug/logging.h>

namespace editor {

	static ImU32 log_severity_to_color(plog::Severity severity) {
		ImU32 color = 0;
		switch (severity) {
			case plog::Severity::verbose:
			case plog::Severity::debug:
				color = IM_COL32(59, 215, 226, 255);
				break;

			case plog::Severity::info:
				color = IM_COL32(0, 0, 0, 255);
				break;

			case plog::Severity::warning:
				color = IM_COL32(255, 216, 96, 255);
				break;

			case plog::Severity::error:
			case plog::Severity::fatal:
				color = IM_COL32(255, 0, 0, 255);
				break;
		}
		return color;
	}

	void update_log_window(
		const std::vector<platform::LogEntry>& log,
		std::vector<EditorCommand>* commands,
		core::Signal<size_t>* last_num_seen_log_entries
	) {
		if (ImGui::Button("Clear")) {
			commands->push_back(EditorCommand::ClearLog);
		}

		ImGui::Spacing();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 255, 255, 255));
		ImGui::BeginChild("LogOutput", ImVec2(0, 0), ImGuiChildFlags_Border);
		{
			// Print log messages
			for (const platform::LogEntry& entry : log) {
				ImGui::PushStyleColor(ImGuiCol_Text, log_severity_to_color(entry.severity));
				ImGui::Text("%s", entry.message.c_str());
				ImGui::PopStyleColor();
			}

			// Auto-scroll on new messages unless scroll position is set by user
			*last_num_seen_log_entries = log.size();
			const float scroll_y = ImGui::GetScrollY();
			const float scroll_max = ImGui::GetScrollMaxY();
			const float text_height = ImGui::GetTextLineHeightWithSpacing();
			const int lines_to_count = 5;
			const bool scrolled_up = scroll_y <= scroll_max - lines_to_count * text_height;
			if (last_num_seen_log_entries->just_changed() && !scrolled_up) {
				ImGui::SetScrollHereY();
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

} // namespace editor
