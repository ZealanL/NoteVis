#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"

#include "../Render/Renderer.h"
#include "UI/Dialog/Dialog.h"
#include "UI/UI.h"
#include "MIDI/MIDI.h"

#include "../Render/Draw/Draw.h"
#include "NVFileSystem/NVFileSystem.h"


NoteGraph::RenderContext GetNoteGraphRenderCtx() {
	NoteGraph::RenderContext ctx;
	ctx.fullNoteGraphScreenArea = Area(
		Vec(0, ImGui::GetFrameHeight()), // Pad for top menu bar
		Renderer::GetWindowSize()
	);
	return ctx;
}

void Core::OnRender() {
	auto renderCtx = GetNoteGraphRenderCtx();
	g_NoteGraph.Render(&renderCtx);
}

void Core::TryUserExit() {
	if (NVFileSystem::TryCloseScore())
		EXIT(EXIT_SUCCESS);
}

void Core::ProcessEvent(SDL_Event& e) {
	if (UI::PreventInput())
		return;

	bool handledByAction = false;

	switch (e.type) {
	case SDL_QUIT:
		return TryUserExit();
	case SDL_KEYDOWN:
	{
		SDL_Keycode pressedKey = e.key.keysym.sym;
		auto keyMods = e.key.keysym.mod;
		BYTE kbFlags = 0;
		kbFlags |= KBFLAG_CTRL * ((keyMods & KMOD_CTRL) > 0);
		kbFlags |= KBFLAG_SHIFT * ((keyMods & KMOD_SHIFT) > 0);
		kbFlags |= KBFLAG_ALT * ((keyMods & KMOD_ALT) > 0);

		for (auto& pair : g_Actions.actions) {
			auto action = pair.second;
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
	Area inputArea = renderCtx.fullNoteGraphScreenArea;
	// Only events to the note graph if they are within the screen area and no ImGui menu is active
	if (g_MousePos >= inputArea.min && g_MousePos <= inputArea.max && !g_WasImGuiMenuActive)
		g_NoteGraph.UpdateWithInput(e, &renderCtx);
}