#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"

#include "Action/ActionDefines.h"
#include "../Render/Renderer.h"

auto GetNoteGraphScreenArea() {
	return Area({ 0,0 }, Renderer::GetWindowSize());
}

void Core::OnRender() {
	g_NoteGraph.Render(GetNoteGraphScreenArea());
}

void Core::OnUserExit() {
	// TODO: Implement unsaved work warning
	EXIT(EXIT_SUCCESS);
}

void Core::ProcessEvent(SDL_Event& e) {

	switch (e.type) {
	case SDL_QUIT:
		return OnUserExit();

	case SDL_WINDOWEVENT:
		if (e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == SDL_GetWindowID(g_SDL_Window))
			return OnUserExit();
		break;

	case SDL_KEYDOWN:
	{
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

		break;
	}
	case SDL_MOUSEMOTION:
		g_MousePos = Vec(e.motion.x, e.motion.y);
		break;
	}

	g_NoteGraph.UpdateWithInput(GetNoteGraphScreenArea(), e);
}
