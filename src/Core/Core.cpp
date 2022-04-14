#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"

#include "Action/ActionDefines.h"
#include "../Render/Renderer.h"
#include "UI/Dialog/Dialog.h"
#include "UI/UI.h"
#include "MIDI/MIDI.h"

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

		for (auto& action : g_Actions) {
			if (action.bind.key == pressedKey && action.bind.flags == kbFlags) {
				if (action.undoable)
					UpdateHistory();

				action.Execute();
			}
		}

		break;
	}
	case SDL_MOUSEMOTION:
		g_MousePos = Vec(e.motion.x, e.motion.y);
		break;
	}

	g_NoteGraph.UpdateWithInput(GetNoteGraphScreenArea(), e);
}

deque<ByteDataStream> noteGraphHistory;

void Core::UpdateHistory() {
	ByteDataStream data;
	g_NoteGraph.Serialize(data);

	if (!noteGraphHistory.empty()) {
		if (noteGraphHistory.front().DataMatches(data)) {
			// Prevent storing history when nothing actually changed
			return;
		}
	}

	noteGraphHistory.push_front(data);

	constexpr size_t MAX_HISTORY_SIZE = (1000 * 1000) * 4; // 4 megabytes

	// NOTE: Could store this as a global variable and update it with any changes instead for efficiency, but this is cleaner and simpler
	size_t totalHistorySize = 0;
	for (auto history : noteGraphHistory)
		totalHistorySize += history.size();

	if (totalHistorySize > MAX_HISTORY_SIZE) {
		DLOG("Undo history size of {}mb exceeds limit of {}mb, removing old history",
			totalHistorySize / (1000.f * 1000.f),
			MAX_HISTORY_SIZE / (1000.f * 1000.f)
		);

		auto& back = noteGraphHistory.back();
		noteGraphHistory.pop_back();
	}
}

bool Core::UndoRestoreHistory() {
	if (!noteGraphHistory.empty()) {
		auto& data = noteGraphHistory.front();
		auto deserializeItr = data.GetIterator();

		g_NoteGraph.ClearEverything();
		g_NoteGraph.Deserialize(deserializeItr);

		DLOG("\tUndo restored {} serialized bytes", data.size());

		noteGraphHistory.pop_front();
		return true;
	} else {
		DLOG("\tNothing left to undo.");
		return false;
	}
}
