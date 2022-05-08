#pragma once
#include "../NoteTypes.h"

// A storage system for efficiency when searching/going through notes
class NoteStorageCache {
public:
	typedef std::function<bool(Note* a, Note* b)> NoteCompareFn;

	NoteStorageCache() {}
	NoteStorageCache(const NoteStorageCache& other) = default;

private:
	// Datastructure syncronization helpers
	void OnNoteAdded(Note* note);
	void OnNoteDeleted(Note* note);

	// Compare functions
	static bool CompareNoteStartTime(Note* a, Note* b);
	static bool CompareNoteEndTime(Note* a, Note* b);
public:

	PRIVCONST(unordered_set<Note*>, notes, {});
	PRIVCONST_ARR(unordered_set<Note*>, slots, KEY_AMOUNT, {});
	PRIVCONST(unordered_set<Note*>, selected, {});

	typedef set<Note*, NoteCompareFn> SortedNoteSet;
	PRIVCONST(SortedNoteSet, sortedByStartTime, SortedNoteSet(CompareNoteStartTime));
	PRIVCONST(SortedNoteSet, sortedByEndTime, SortedNoteSet(CompareNoteEndTime));

public:

	NoteTime GetFurthestNoteEndTime() {
		if (notes.empty())
			return 0;

		Note* last = *std::prev(_sortedByEndTime.end());
		return last->time + last->duration;
	}

	// Update functions
	void OnNoteChanged(Note* note, bool potentialKeyChange);
	void SetSelected(Note* note, bool selected);
	void DeselectAll() { _selected.clear(); }
	Note* AddNote(Note note);
	bool RemoveNote(Note* note); // Returns false if not found

	void Reset() { 
		// Free memory
		for (Note* note : _notes)
			delete note;
		_notes.clear();

		_selected.clear();
		_sortedByEndTime.clear();
		_sortedByStartTime.clear();
		for (auto& slot : _slots)
			slot.clear();
	}

	// For C++ iterator
	auto begin() const { return notes.begin(); }
	auto end() const { return notes.end(); }

	~NoteStorageCache() {
		// Free memory
		for (Note* note : _notes)
			delete note;
		_notes.clear();
	}
};