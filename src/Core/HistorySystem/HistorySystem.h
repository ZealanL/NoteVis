#pragma once
#include "../../Types/ByteDataSteam.h"

struct HistoryState {
	double time;
	ByteDataStream graphData;

	bool Matches(HistoryState other) {
		return this->graphData.DataMatches(other.graphData);
	}

	size_t GetSize() {
		return graphData.size();
	}
};

class HistorySystem {
private:

	deque<HistoryState> states;

	// Moved from states, for redoing
	deque<HistoryState> redoStates;
public:

	// Serialize the current state to history - will erase anything above the current undo depth
	// Returns true if a new state was created
	bool Update(bool ignoreTime = false);

	// Try to restore previous serialized state
	// Returns true if history was restored, false if no history left to undo
	bool Undo();

	// Try to restore next serialized state
	// Returns true if history was restored, false if no history left to redo
	bool Redo();
};
