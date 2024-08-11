#include <platform/debug/logging.h>

#include <platform/os/lean_mean_windows.h>

#include <algorithm>
#include <cstring>
#include <debugapi.h>
#include <iomanip>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/MessageOnlyFormatter.h>
#include <plog/Init.h>
#include <vector>

static std::vector<platform::LogEntry> g_log_entries;

namespace plog {

	std::string pretty_file_name(std::string file_name) {
		size_t src_pos = file_name.find("src");
		std::string substr = file_name.substr(src_pos + 4);
		std::replace(substr.begin(), substr.end(), '\\', '/');
		return substr;
	}

	class SimpleTxtFormatter {
	public:
		static util::nstring header() {
			return {};
		}

		static util::nstring format(const Record& record) {
			tm t;
			util::localtime_s(&t, &record.getTime().time);

			util::nostringstream ss;
			ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int>(record.getTime().millitm) << PLOG_NSTR(" ");
			ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity());
			ss << PLOG_NSTR("[") << pretty_file_name(record.getFile()).c_str() << PLOG_NSTR(":") << record.getLine() << PLOG_NSTR("] ");
			ss << record.getMessage() << PLOG_NSTR("\n");

			std::string str = ss.str();

			// save in-memory copy of log message
			g_log_entries.push_back(platform::LogEntry { .message = str, .severity = record.getSeverity() });

			// log to Visual Studio debug log
			OutputDebugStringA(str.c_str());

			return str;
		}
	};

} // namespace plog

namespace platform {

	void init_logging() {
		static plog::ColorConsoleAppender<plog::SimpleTxtFormatter> consoleAppender(plog::streamStdOut);
		plog::init(plog::verbose, &consoleAppender);
	}

	void init_test_logging() {
		static plog::ConsoleAppender<plog::MessageOnlyFormatter> consoleAppender(plog::streamStdErr);
		plog::init(plog::verbose, &consoleAppender);
	}

	const std::vector<LogEntry>& get_in_memory_log() {
		return g_log_entries;
	}

	void clear_in_memory_log() {
		g_log_entries.clear();
	}

} // namespace platform
