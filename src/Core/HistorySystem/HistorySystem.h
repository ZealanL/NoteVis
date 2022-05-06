#pragma once
#include "../../Types/ByteDataSteam.h"

class HistoryState {
private:
	FW::HASH hash;
public:
	double time;
	ByteDataStream graphData;

	HistoryState(ByteDataStream graphData, double time = CURRENT_TIME) {
		this->time = CURRENT_TIME;
		this->graphData = graphData;
		this->hash = graphData.CalculateHash();
	}

	bool Matches(HistoryState other) {
		return this->hash == other.hash;
	}

	size_t GetSize() {
		return graphData.size();
	}

	FW::HASH GetHash() {
		return hash;
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
