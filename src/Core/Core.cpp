#include "Core.h"

#include "Note/NoteGraph/NoteGraph.h"

#include "Action/ActionDefines.h"
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
		for (auto& action : g_Actions) {
			if (action.bind.key == pressedKey && action.bind.flags == kbFlags) {
				if (action.undoable)
					UpdateHistory();

				action.Execute();
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

#pragma region Undo History
struct HistoryState {
	double time;
	ByteDataStream graphData;

	size_t GetSize() {
		return graphData.size();
	}
};
deque<HistoryState> historyStates;

void Core::UpdateHistory() {
	// Minimum amount of time between history updates, this will "merge" multiple history changes into one if they happen quickly
	constexpr double MIN_HISTORY_UPDATE_DELAY = 3;

	static double lastUpdateTime = 0;
	if (CURRENT_TIME >= lastUpdateTime + MIN_HISTORY_UPDATE_DELAY) {
		lastUpdateTime = CURRENT_TIME;
	} else {
		// Hasn't been long enough
		return;
	}

	ByteDataStream data;
	g_NoteGraph.Serialize(data);

	if (!historyStates.empty()) {
		if (historyStates.front().graphData.DataMatches(data)) {
			// Prevent storing history when nothing actually changed
			return;
		}
	}

	historyStates.push_front(HistoryState(CURRENT_TIME, data));

	size_t MAX_HISTORY_SIZE = (1000 * 1000) * g_ARG_MaxHistoryMemSize;

	// NOTE: Could store this as a global variable and update it with any changes instead for efficiency, but this is cleaner and simpler
	size_t totalHistorySize = 0;
	for (auto history : historyStates)
		totalHistorySize += history.GetSize();

	bool firstLoop = true;
	while (historyStates.size() > 1 && totalHistorySize > MAX_HISTORY_SIZE) {

		if (firstLoop) {
			DLOG("Undo history size of {}mb exceeds limit of {}mb, removing old history",
				totalHistorySize / (1000.f * 1000.f),
				MAX_HISTORY_SIZE / (1000.f * 1000.f)
			);
		}

		firstLoop = false;

		// Remove one history state - find best to remove based off of lowest delta time from next state
		auto bestToRemove = historyStates.begin();
		double lowestTimeDelta = DBL_MAX, lastTime = historyStates.begin()->time;
		for (auto itr = std::next(historyStates.begin()); itr != historyStates.end(); itr++) {
			auto& state = *itr;
			
			double timeDelta = abs(state.time - lastTime);
			if (timeDelta < lowestTimeDelta) {
				lowestTimeDelta = timeDelta;
				bestToRemove = itr;
			}

			lastTime = state.time;
		}

		DLOG("\tRemoving history state from {} ago (had time delta of {})",
			FW::TimeDurationToString(CURRENT_TIME - bestToRemove->time),
			FW::TimeDurationToString(lowestTimeDelta)
		);

		totalHistorySize -= bestToRemove->GetSize();
		historyStates.erase(bestToRemove);
	}
}

bool Core::UndoRestoreHistory() {
	if (!historyStates.empty()) {
		auto& data = historyStates.front();
		auto deserializeItr = data.graphData.GetIterator();

		g_NoteGraph.ClearEverything();
		g_NoteGraph.Deserialize(deserializeItr);

		DLOG("\tUndo restored {} serialized bytes", data.graphData.size());

		historyStates.pop_front();
		return true;
	} else {
		DLOG("\tNothing left to undo.");
		return false;
	}
}
#pragma endregion