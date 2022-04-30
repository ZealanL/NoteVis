#pragma once
#include "../Framework.h"
#include "../Globals.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"
#include "Action/Defines/ActionDefineSystem.h"
#include "Note/NoteGraph/NoteGraph.h"
#include "../Types/StaticList.h"

// Add a log notification to the global notegraph
#define NG_NOTIF(s, ...) g_NoteGraph.logNotifs.Add(std::format(s, ##__VA_ARGS__))

namespace Core {
	void OnRender();

	void OnUserExit();

	void ProcessEvent(SDL_Event& e);
}