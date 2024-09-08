#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <platform/debug/logging.h>

#include <stddef.h>
#include <vector>

namespace editor {

	class LogWindow {
	public:
		void update(
			const std::vector<platform::LogEntry>& log,
			std::vector<EditorCommand>* commands
		);

	private:
		core::Signal<size_t> m_num_log_entries = 0;
	};

} // namespace editor
