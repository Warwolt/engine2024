#pragma once

#include <crtdbg.h>
#include <platform/logging.h>
#include <stdio.h>
#include <stdlib.h>

#define ABORT(...)                                           \
	do {                                                     \
		char _error_msg[256];                                \
		int _offset = sprintf_s(_error_msg, 256, "ABORT: "); \
		sprintf_s(_error_msg + _offset, 256, __VA_ARGS__);    \
		LOG_ERROR(_error_msg);                               \
		__debugbreak();                                      \
		exit(1);                                             \
	} while (0)

#define ASSERT(expr, ...)                                                       \
	if (!(expr)) {                                                              \
		char _error_msg[256];                                                   \
		int _offset = sprintf_s(_error_msg, 256, "ASSERT(%s) failed: ", #expr); \
		sprintf_s(_error_msg + _offset, 256, __VA_ARGS__);                       \
		LOG_ERROR(_error_msg);                                                  \
		__debugbreak();                                                         \
		exit(1);                                                                \
	}
