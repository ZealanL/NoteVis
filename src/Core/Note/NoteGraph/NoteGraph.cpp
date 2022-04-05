#include "NoteGraph.h"


Vec NoteGraph::ToScreenPos(NoteTime time, float y, Vec screenSize) {
	int relativeTime = time - hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	float outY = -((y - (KEY_AMOUNT_SUB1_F /2.f)) / KEY_AMOUNT_SUB1_F) * vScale;
	return Vec(outX + screenSize.x/2, outY + screenSize.y/2);
}

float NoteGraph::ScreenYToGraphY(float screenY, Vec screenSize) {
	float decenteredY = screenY - screenSize.y / 2.f;
	
	// Didn't feel like doing algebra today so:
	// https://www.wolframalpha.com/input?i=x+%3D+-%28%28y+-+%28a+%2F2%29%29+%2F+a%29+*+v%2C+solve+for+y

	return (KEY_AMOUNT_SUB1_F * (vScale - 2*decenteredY)) / (2*vScale);
}

NoteTime NoteGraph::ScreenXToNoteTime(float screenX, Vec screenSize) {
	float decenteredX = screenX - screenSize.x / 2.f;
	return hScroll + (1000.f * decenteredX) / hZoom;
}

Note* NoteGraph::AddNote(Note note) {
	Note* newNote = new Note(note);
	_notes.insert(newNote);

	// Update _furthestNoteEndTime
	NoteTime endTime = newNote->time + newNote->duration;
	_furthestNoteEndTime = MAX(_furthestNoteEndTime, endTime);

	return newNote;
}

bool NoteGraph::RemoveNote(Note* note) {

	if (note == hoveredNote) {
		hoveredNote = NULL;
	}

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
	hoveredNote = NULL;
}

int NoteGraph::GetNoteCount() {
	return _notes.size();
}