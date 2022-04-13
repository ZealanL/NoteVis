#pragma once
#include "../Framework.h"
#include "../Globals.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"
#include "Action/Action.h"
#include "Note/NoteGraph/NoteGraph.h"
#include "../Types/StaticList.h"

namespace Core {
	void OnRender();

	void OnUserExit();

	void ProcessEvent(SDL_Event& e);
}