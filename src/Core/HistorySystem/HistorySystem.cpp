#include "HistorySystem.h"

#include "../../Globals.h"
#include "../Core.h"

HistoryState GetCurrentState() {
	ByteDataStream data;
	g_NoteGraph.Serialize(data);
	return HistoryState(data);
}

bool HistorySystem::Update(bool ignoreTime) {
	auto currentState = GetCurrentState();

	// Minimum amount of time between history updates, this will "merge" multiple history changes into one if they happen quickly
	constexpr double MIN_HISTORY_UPDATE_DELAY = 2;
	static double lastUpdateTime = -DBL_MAX;
	if (CURRENT_TIME >= lastUpdateTime + MIN_HISTORY_UPDATE_DELAY || ignoreTime) {
		lastUpdateTime = CURRENT_TIME;
	} else {
		// Hasn't been long enough, 
		return false;
	}

	if (!states.empty() && states.front().Matches(currentState))
		return false; // No change

	// If we were undoing things, remove redo states
	if (!redoStates.empty()) {
		DLOG("Removing {} redo states due to new history update", redoStates.size());
		redoStates.clear();
	}

	DLOG("Adding history entry (size = {}mb), hash: {:X}", currentState.GetSize() / (1000.f * 1000.f), currentState.GetHash());
	states.push_front(currentState);

	size_t MAX_HISTORY_SIZE = (1000 * 1000) * g_ARG_MaxHistoryMemSize;

	// NOTE: Could store this as a global variable and update it with any changes instead for efficiency, but this is cleaner and simpler
	size_t totalHistorySize = 0;
	for (auto history : states)
		totalHistorySize += history.GetSize();

	bool firstLoop = true;
	while (states.size() > 1 && totalHistorySize > MAX_HISTORY_SIZE) {

		if (firstLoop) {
			DLOG("Undo history size of {}mb exceeds limit of {}mb, removing old history",
				totalHistorySize / (1000.f * 1000.f),
				MAX_HISTORY_SIZE / (1000.f * 1000.f)
			);
		}

		firstLoop = false;

		// Remove one history state - find best to remove based off of lowest delta time from next state
		auto bestToRemove = states.begin();
		double lowestTimeDelta = DBL_MAX, lastTime = states.begin()->time;
		for (auto itr = std::next(states.begin()); itr != states.end(); itr++) {
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
		states.erase(bestToRemove);
	}

	return true;
}

bool HistorySystem::Undo() {
	auto currentState = GetCurrentState();

	if (!states.empty() && states.front().GetHash() == currentState.GetHash())
		states.pop_front();

	if (!states.empty()) {
		bool addedCurrent = false;

		if (redoStates.empty()) {

			// If we don't have any undo depth, add the current state to redo states (if not matching)
			if (!currentState.Matches(states.front())) {
				DLOG("Forced current state to be added to history");
				redoStates.push_front(currentState);
				addedCurrent = true;
			}
		}

		// Move the state we undid to into redoStates
		if (!addedCurrent)
			redoStates.push_front(currentState);

		auto deserializeItr = states.front().graphData.GetIterator();

		g_NoteGraph.ClearEverything(false);
		g_NoteGraph.Deserialize(deserializeItr);

		states.pop_front();

		NG_NOTIF("Undid changes.");
		DLOG("Undid changes, undo state count = {}, redo state count = {}", states.size(), redoStates.size());
		return true;
	} else {
		NG_NOTIF("Nothing left to undo.");
		return false;
	}
}

bool HistorySystem::Redo() {
	if (!redoStates.empty()) {
		states.push_front(GetCurrentState());

		auto deserializeItr = redoStates.front().graphData.GetIterator();
		g_NoteGraph.ClearEverything(false);
		g_NoteGraph.Deserialize(deserializeItr);

		redoStates.pop_front();

		NG_NOTIF("Redid changes.");
		DLOG("Redid changes, undo state count = {}, redo state count = {}", states.size(), redoStates.size());
		return true;
	} else {
		NG_NOTIF("Nothing left to redo.");
		return false;
	}
}