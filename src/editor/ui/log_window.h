#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <platform/debug/logging.h>

#include <stddef.h>
#include <vector>

namespace editor {

	struct LogWindow {
		core::Signal<size_t> num_log_entries = 0;
	};

	void update_log_window(
		LogWindow* log_window,
		const std::vector<platform::LogEntry>& log,
		std::vector<EditorCommand>* commands
	);

} // namespace editor
