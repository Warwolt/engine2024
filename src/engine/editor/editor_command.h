#pragma once

namespace engine {

	enum class EditorCommand {
		// file
		NewProject,
		OpenProject,
		SaveProject,
		SaveProjectAs,
		// mouse
		SetCursorToSizeAll,
		SetCursorToArrow,
		// game
		RunGame,
		ResetGameState,
		// app
		Quit,
	};

} // namespace engine
