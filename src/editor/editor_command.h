#pragma once

namespace editor {

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
		ClearLog,
		Quit,
	};

} // namespace editor
