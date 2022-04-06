#include "NoteGraph.h"


float NoteGraph::GetAreaScreenHeight(Area screenArea) {
	return vScale * (screenArea.GetHeight() - (screenArea.GetHeight() / KEY_AMOUNT) * 2);
}

Vec NoteGraph::ToScreenPos(GraphPos graphPos, Area screenArea) {
	int relativeTime = graphPos.x - hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	float outY = -((graphPos.y - (KEY_AMOUNT_SUB1_F /2.f)) / KEY_AMOUNT_SUB1_F) * GetAreaScreenHeight(screenArea);
	return Vec(outX, outY) + screenArea.GetCenter();
}

GraphPos NoteGraph::ToGraphPos(Vec screenPos, Area screenArea) {
	screenPos -= screenArea.GetCenter();

	NoteTime x = hScroll + (NoteTime)((1000.f * screenPos.x) / hZoom);

	float height = GetAreaScreenHeight(screenArea);

	// Didn't feel like doing algebra today so:
	// https://www.wolframalpha.com/input?i=x+%3D+-%28%28y+-+%28a+%2F2%29%29+%2F+a%29+*+v%2C+solve+for+y
	float y = (KEY_AMOUNT_SUB1_F * (height - 2 * screenPos.y)) / (2 * height);

	return { x, y };
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