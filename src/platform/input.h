#pragma once

namespace platform {

	struct Input {
		bool quit_signal_received;
		bool escape_key_pressed;
	};

	Input read_input();

} // namespace platform
