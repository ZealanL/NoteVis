#pragma once
#include "../NoteTypes.h"
#include "../../Types/Vec.h"

struct TimeSignature {
	int num = 4;
	// TODO: Denominator so we can support 7/8 and such
};

// NOTE: Rendering of the NoteGraph is Rendering/NoteGraphRender
class NoteGraph {
private:
	// Private to prevent manual removal
	set<Note*> _notes;
public:
	
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

	// y: represents key slot coordinate, can have decimal value obviously
	// screenSize: needed for centering
	Vec ToScreenPos(NoteTime time, float y, Vec screenSize);

	int GetNoteCount();
	const set<Note*>& GetNotes() { return _notes; }

	Note* AddNote(Note note); // Returns pointer to added note
	bool AddNote(Note* note); // Returns true if note was added, false if already present in note set
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