#pragma once
#include "../NoteTypes.h"

struct TimeSignature {
	int num = 4;
	// TODO: Denominator so we can support 7/8 and such
};

struct GraphPos {
	NoteTime x;
	float y;
};

// NOTE: Rendering of the NoteGraph is Rendering/NoteGraphRender
class NoteGraph {
private:
	// Private to prevent manual modification, this would cause serious desyncronization issues
	set<Note*> _notes;
	NoteTime _furthestNoteEndTime;

	// Keep track of what notes are in what key slot for fast comparisons
	set<Note*> _noteSlots[KEY_AMOUNT];
public:
	// Current mode
	enum {
		MODE_IDLE,
		MODE_RECTSELECT,
		MODE_PLAY
	};
	int currentMode = MODE_IDLE;

	struct NoteGraphModeInfo {
		GraphPos startDragPos;
	};
	NoteGraphModeInfo modeInfo;

	NoteTime GetFurthestNoteEndTime() {
		return _furthestNoteEndTime;
	}

	Note* hoveredNote = NULL;

	set<Note*> selectedNotes; // Only contains pointers already in _notes 
	bool IsNoteSelected(Note* note) {
		return selectedNotes.find(note) != selectedNotes.end();
	}

	TimeSignature timeSig;

	// Horizontal scroll of the note graph
	NoteTime hScroll = 0;

	// Horizontal zoom, Pixels per NOTETIME_PER_BEAT NoteTime
	float hZoom = 100.f;

	// Vertical scale/zoom, how high the full notegraph is relative to screen height 
	float vScale = 1.f;

	float GetNoteAreaScreenHeight(Area screenArea);
    Vec ToScreenPos(GraphPos graphPos, Area screenArea);
	GraphPos ToGraphPos(Vec screenPos, Area screenArea);

	// Fix any potential overlap with this note, will only modify/remove other notes
    void CheckFixNoteOverlap(Note* note);

	// ignoreOverlap: Don't call CheckFixNoteOverlap after moving
	void MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap = false);

	int GetNoteCount();
	const set<Note*>& GetNotes() { return _notes; }
	Note* AddNote(Note note); // Returns pointer to added note
	bool RemoveNote(Note* note); // Returns true if note was found and removed
	void ClearNotes();

	void Render(Area screenArea);
	void UpdateWithInput(Area screenArea, SDL_Event& e);

	// Returns true if they could be moved, false if not
	bool TryMoveSelectedNotes(int amountX, int amountY);

	// Prevent memory leak
	~NoteGraph() {
		for (Note* note : _notes) {
			delete note;
		}
	}

	// For C++ iterator
	auto begin() { return _notes.begin(); }
	auto end() { return _notes.end(); }
};