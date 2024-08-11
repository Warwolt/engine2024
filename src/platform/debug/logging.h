#pragma once

#define PLOG_OMIT_LOG_DEFINES
#define PLOG_CAPTURE_FILE
#include <plog/Log.h>

#include <string>
#include <vector>

// define printf style logging macros
#define LOG_VERBOSE(...) PLOG_VERBOSE.printf(__VA_ARGS__)
#define LOG_DEBUG(...) PLOG_DEBUG.printf(__VA_ARGS__)
#define LOG_INFO(...) PLOG_INFO.printf(__VA_ARGS__)
#define LOG_WARNING(...) PLOG_WARNING.printf(__VA_ARGS__)
#define LOG_ERROR(...) PLOG_ERROR.printf(__VA_ARGS__)
#define LOG_FATAL(...) PLOG_FATAL.printf(__VA_ARGS__)
#define LOG_NONE(...) PLOG_NONE.printf(__VA_ARGS__)
#define LOG(level, ...) PLOG(level).printf(__VA_ARGS__)

namespace platform {

	struct LogEntry {
		std::string message;
		plog::Severity severity;
	};

	void init_logging();
	void init_test_logging();
	const std::vector<LogEntry>& get_in_memory_log();
	void clear_in_memory_log();

} // namespace platform
