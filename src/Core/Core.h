#pragma once
#include "../Framework.h"
#include "Types/Vec.h"
#include "Types/Color.h"
#include "Action/Action.h"
#include "Note/NoteGraph/NoteGraph.h"

// Globals
inline SDL_Window* g_SDL_Window;
inline SDL_GLContext g_SDL_GLContext;

inline NoteGraph g_NoteGraph;
inline vector<Action> g_Actions;

namespace Core {
	void OnRender();

	void OnUserExit();

	void ProcessEvent(SDL_Event& e);

	FINLINE Vec GetWindowSize() {
		int w, h;
		SDL_GetWindowSize(g_SDL_Window, &w, &h);
		return Vec(w, h);
	}
}