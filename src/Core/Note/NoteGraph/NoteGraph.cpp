#include "NoteGraph.h"

Vec NoteGraph::ToScreenPos(NoteTime time, float y, Vec screenSize) {
	int relativeTime = time - this->hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	float adjustedKeyAmount = KEY_AMOUNT - 1;

	float outY = -((y - (adjustedKeyAmount /2.f)) / adjustedKeyAmount) * vScale;
	return Vec(outX + screenSize.x/2, outY + screenSize.y/2);
}

Note* NoteGraph::AddNote(Note note) {
	Note* newNote = new Note(note);
	_notes.insert(newNote);
	return newNote;
}

bool NoteGraph::AddNote(Note* note) {
	return _notes.insert(note).second;
}

bool NoteGraph::RemoveNote(Note* note) {
	selectedNotes.erase(note);
	bool found = _notes.erase(note);
	if (found)
		delete note;
	return found;
}

void NoteGraph::ClearNotes() {
	for (Note* note : _notes) {
		delete note;
	}

	_notes.clear();
	selectedNotes.clear();
}

int NoteGraph::GetNoteCount() {
	return _notes.size();
}