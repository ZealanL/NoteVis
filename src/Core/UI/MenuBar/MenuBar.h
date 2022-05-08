#pragma once

enum class MenuBarSection {
	INVALID = -1,

	FILE, EDIT, NOTEGRAPH, NOTES, NOTES_SELECTEDONLY
};

namespace MenuBar {
	void Draw();
}