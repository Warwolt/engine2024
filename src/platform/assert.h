#pragma once

#include <platform/logging.h>
#include <stdlib.h>

#include <crtdbg.h>

#define ASSERT(expr, ...)       \
	if (!(expr)) {              \
		LOG_ERROR(__VA_ARGS__); \
		__debugbreak();         \
		exit(1);                \
	}
