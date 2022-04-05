#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"
#include "Note/NoteGraph/NoteGraphRenderer/NoteGraphRenderer.h"

#include "Action/ActionDefines.h"

void Core::OnRender() {
	g_NoteGraph.vScale = GetWindowSize().y - (GetWindowSize().y / KEY_AMOUNT)*2; // A bit of padding
	NoteGraphRenderer::DrawNoteGraph(g_NoteGraph, Area({ 0,0 }, GetWindowSize()));
}

void Core::OnUserExit() {
	// TODO: Implement unsaved work warning
	EXIT(EXIT_SUCCESS);
}

void Core::ProcessEvent(SDL_Event& e) {
	if (e.type == SDL_QUIT)
		return OnUserExit();
	if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == SDL_GetWindowID(g_SDL_Window))
		return OnUserExit();

	auto keyboardState = SDL_GetKeyboardState(NULL);
	bool isShiftDown = keyboardState[SDL_SCANCODE_RSHIFT] || keyboardState[SDL_SCANCODE_LSHIFT];
	bool isCntrolDown = keyboardState[SDL_SCANCODE_RCTRL] || keyboardState[SDL_SCANCODE_LCTRL];

	if (e.type == SDL_KEYDOWN) {
		SDL_Keycode pressedKey = e.key.keysym.sym;
		auto keyMods = e.key.keysym.mod;
		BYTE kbFlags = 0;
		kbFlags |= KBFLAG_CTRL * ((keyMods & KMOD_CTRL) > 0);
		kbFlags |= KBFLAG_SHIFT * ((keyMods & KMOD_SHIFT) > 0);
		kbFlags |= KBFLAG_ALT * ((keyMods & KMOD_ALT) > 0);

		for (auto& action : g_Actions) {
			if (action.bind.key == pressedKey && action.bind.flags == kbFlags)
				action.Execute();
		}
	}

	// Scroll note graph
	if (e.type == SDL_MOUSEWHEEL) {
		g_NoteGraph.hScroll += -e.wheel.y * (isShiftDown ? 5000 : 500);
		g_NoteGraph.hScroll = CLAMP(g_NoteGraph.hScroll, 0, g_NoteGraph.GetFurthestNoteEndTime());
	}

	if (e.type == SDL_MOUSEMOTION) {
		g_MousePos = Vec(e.motion.x, e.motion.y);
	}
}
