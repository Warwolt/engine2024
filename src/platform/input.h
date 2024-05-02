#pragma once

#include <stdint.h>

namespace platform {

	struct Input {
		uint64_t delta_ms;
		bool quit_signal_received;
		bool escape_key_pressed;
	};

	Input read_input();

} // namespace platform
