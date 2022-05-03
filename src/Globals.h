#pragma once
// Global variables are defined in here

#include "Framework.h"
#include "Types/Vec.h"
#include "Core/HistorySystem/HistorySystem.h"
#include "Core/Action/Defines/ActionDefineSystem.h"
#include "Core/Note/NoteGraph/NoteGraph.h"

#include "Core/UI/Dialog/Dialog.h"

// Should not include anything aside from framework and the types it stores

#define GBLVAR(type, name)			inline type g_##name = {};
#define GBLVAR_V(type, name, value) inline type g_##name = value;

#pragma region Global Variables

#pragma region Command Line Args
// Maximum undo history memory size, in megabytes
GBLVAR_V(int, ARG_MaxHistoryMemSize, 100);

// Developer mode
GBLVAR_V(bool, ARG_DevMode, false);
#pragma endregion

GBLVAR(SDL_Renderer*,	SDL_Renderer);
GBLVAR(SDL_Window*,		SDL_Window);
GBLVAR(SDL_GLContext,	SDL_GLContext);

GBLVAR(Vec, MousePos);
GBLVAR(ActionDefineSystem, Actions);

// TODO: Make proper history system interface (?)
GBLVAR(HistorySystem, History)

GBLVAR(NoteGraph, NoteGraph);

GBLVAR(bool, KeyboardState[SDL_NUM_SCANCODES], {});
GBLVAR(bool, MouseState[6], {});

#pragma region Global Hold Actions
#pragma endregion

#pragma endregion
#undef GBLVAR