#pragma once
#include "../Framework.h"
#include "../Globals.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"
#include "Action/Action.h"
#include "Note/NoteGraph/NoteGraph.h"
#include "../Types/StaticList.h"

// Add a log notification to the global notegraph
#define NG_NOTIF(s, ...) g_NoteGraph.logNotifs.Add(std::format(s, ##__VA_ARGS__))

namespace Core {
	void OnRender();

	void OnUserExit();

	void ProcessEvent(SDL_Event& e);

	// Add the g_NoteGraph's serialize data to g_NoteGraphHistory
	void UpdateHistory();

	// Restore (and pop) last serialized data from g_NoteGraphHistory to g_NoteGraph
	// Returns true if history was restored, false if no history
	bool UndoRestoreHistory();
}