#pragma once
#include "../Framework.h"
#include "../Globals.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"
#include "Action/Action.h"
#include "Note/NoteGraph/NoteGraph.h"

namespace Core {
	void OnRender();

	void OnUserExit();

	void ProcessEvent(SDL_Event& e);

	inline Vec GetWindowSize() {
		int w, h;
		SDL_GetWindowSize(g_SDL_Window, &w, &h);
		return Vec(w, h);
	}
}