#pragma once

#define PLOG_OMIT_LOG_DEFINES
#define PLOG_CAPTURE_FILE
#include <plog/Log.h>

#define LOG_INFO(...) PLOG_INFO.printf(__VA_ARGS__)

namespace platform {

	void init_logging();
	void init_test_logging();

} // namespace platform
