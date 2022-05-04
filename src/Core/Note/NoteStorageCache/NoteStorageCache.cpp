#include "NoteStorageCache.h"

bool NoteStorageCache::CompareNoteStartTime(Note* a, Note* b) {
	if (a->time != b->time) {
		return a->time < b->time;
	} else {
		return a->Compare(*b);
	}
}

bool NoteStorageCache::CompareNoteEndTime(Note* a, Note* b) {
	NoteTime aEnd = (a->time + a->duration), bEnd = (b->time + b->duration);

	if (aEnd != bEnd) {
		return aEnd < bEnd;
	} else {
		return a->Compare(*b);
	}
}

void NoteStorageCache::OnNoteAdded(Note* note) {
	_slots[note->key].insert(note);
	_sortedByEndTime.insert(note);
	_sortedByStartTime.insert(note);
}

void NoteStorageCache::OnNoteDeleted(Note* note) {
	_sortedByEndTime.erase(note);
	_sortedByStartTime.erase(note);
	_slots[note->key].erase(note);
	_selected.erase(note);
}

void NoteStorageCache::OnNoteChanged(Note* note, bool potentialKeyChange) {
	if (potentialKeyChange) {
		int timesErased = 0;
		for (auto& slot : _slots)
			timesErased += slot.erase(note);

		ASSERT(timesErased == 1);
	}

	if (potentialKeyChange) {
		_slots[note->key].insert(note);
	}

	_sortedByEndTime.erase(note);
	_sortedByStartTime.erase(note);
	_sortedByEndTime.insert(note);
	_sortedByStartTime.insert(note);
}

void NoteStorageCache::SetSelected(Note* note, bool isSelected) {
	if (isSelected)
		_selected.insert(note);
	else
		_selected.erase(note);
}

Note* NoteStorageCache::AddNote(Note note) {
	ASSERT(note.IsValid());

	Note* newNote = new Note(note);
	_notes.insert(newNote);
	OnNoteAdded(newNote);
	return newNote;
}

bool NoteStorageCache::RemoveNote(Note* note) {
	if (_notes.erase(note)) {
		OnNoteDeleted(note);
		delete note;
		return true;
	} else {
		return false;
	}
}