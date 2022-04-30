#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"

#include "../Render/Renderer.h"
#include "UI/Dialog/Dialog.h"
#include "UI/UI.h"
#include "MIDI/MIDI.h"

NoteGraph::RenderContext GetNoteGraphRenderCtx() {
	NoteGraph::RenderContext ctx;
	ctx.fullNoteGraphScreenArea = Area(
		0,
		Renderer::GetWindowSize()
	);
	return ctx;
}

void Core::OnRender() {
	auto renderCtx = GetNoteGraphRenderCtx();
	g_NoteGraph.Render(&renderCtx);
}

void Core::OnUserExit() {
	// TODO: Implement unsaved work warning
	EXIT(EXIT_SUCCESS);
}

void Core::ProcessEvent(SDL_Event& e) {
	if (UI::PreventInput())
		return;

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

		bool handledByAction = false;
		for (Action* action : g_Actions.actions) {
			if (action->bind.key == pressedKey && action->bind.flags == kbFlags) {
				if (action->undoable)
					g_History.Update();

				action->Execute();
				handledByAction = true;
			}
		}

		if (!handledByAction)
			g_NoteGraph.TryHandleSpecialKeyEvent(pressedKey, kbFlags);

		break;
	}
	case SDL_MOUSEMOTION:
		g_MousePos = Vec(e.motion.x, e.motion.y);
		break;
	}

	auto renderCtx = GetNoteGraphRenderCtx();
	g_NoteGraph.UpdateWithInput(e, &renderCtx);
}