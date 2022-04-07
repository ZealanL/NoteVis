#pragma once
// Global variables are defined in here

#include "Framework.h"
#include "Types/Vec.h"
#include "Core/Action/Action.h"
#include "Core/Note/NoteGraph/NoteGraph.h"

// Should not include anything aside from framework and the types it stores

#define GBLVAR(type, name) inline type g_##name = {};
#pragma region Global Variables

GBLVAR(SDL_Renderer*,	SDL_Renderer);
GBLVAR(SDL_Window*,		SDL_Window);
GBLVAR(SDL_GLContext,	SDL_GLContext);

GBLVAR(Vec, MousePos);
GBLVAR(vector<Action>,	Actions);

GBLVAR(NoteGraph, NoteGraph);
GBLVAR(const BYTE*, KeyboardState);

#pragma endregion
#undef GBLVAR