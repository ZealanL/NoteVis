#pragma once
// Global variables are defined in here

#include "Framework.h"
#include "Types/Vec.h"
#include "Core/Action/Action.h"
#include "Core/Note/NoteGraph/NoteGraph.h"

#include "Core/UI/Dialog/Dialog.h"

// Should not include anything aside from framework and the types it stores

#define GBLVAR(type, name)			inline type g_##name = {};
#define GBLVAR_V(type, name, value) inline type g_##name = value;

#pragma region Global Variables

GBLVAR(SDL_Renderer*,	SDL_Renderer);
GBLVAR(SDL_Window*,		SDL_Window);
GBLVAR(SDL_GLContext,	SDL_GLContext);

GBLVAR(Vec, MousePos);
GBLVAR(vector<Action>,	Actions);

GBLVAR(NoteGraph, NoteGraph);

GBLVAR(bool, KeyboardState[SDL_NUM_SCANCODES], {});
GBLVAR(bool, MouseState[6], {});

#pragma endregion
#undef GBLVAR