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
public:
	
	// Current mode
	enum {
		MODE_IDLE,
		MODE_RECTSELECT,
		MODE_PLAY
	};
	int currentMode = MODE_IDLE;

	struct {
		
	} modeInfo;

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

	// Vertical scale/zoom, how high the full notegraph is 
	float vScale = 1000.f;

	// Convert a graphPos into a screen position
	// screenSize: needed for centering
    Vec ToScreenPos(GraphPos graphPos, Vec screenSize);
	GraphPos ToGraphPos(Vec screenPos, Vec screenSize);

	int GetNoteCount();
	const set<Note*>& GetNotes() { return _notes; }

	Note* AddNote(Note note); // Returns pointer to added note
	bool RemoveNote(Note* note); // Returns true if note was found and removed

	void ClearNotes();

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