#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <platform/debug/logging.h>

#include <stddef.h>
#include <vector>

namespace editor {

	void update_log_window(
		const std::vector<platform::LogEntry>& log,
		std::vector<EditorCommand>* commands,
		core::Signal<size_t>* last_num_seen_log_entries
	);

} // namespace editor
