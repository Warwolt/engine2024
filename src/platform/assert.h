#pragma once

#include <crtdbg.h>
#include <platform/logging.h>
#include <stdio.h>
#include <stdlib.h>

#define EXIT(...)                                                  \
	do {                                                           \
		char error_msg[256];                                       \
		int offset = sprintf_s(error_msg, 256, "EXIT() called: "); \
		sprintf_s(error_msg + offset, 256, __VA_ARGS__);           \
		LOG_ERROR(error_msg);                                      \
		__debugbreak();                                            \
		exit(1);                                                   \
	} while (0)

#define ASSERT(expr, ...)                                                     \
	if (!(expr)) {                                                            \
		char error_msg[256];                                                  \
		int offset = sprintf_s(error_msg, 256, "ASSERT(%s) failed: ", #expr); \
		sprintf_s(error_msg + offset, 256, __VA_ARGS__);                      \
		LOG_ERROR(error_msg);                                                 \
		__debugbreak();                                                       \
		exit(1);                                                              \
	}
