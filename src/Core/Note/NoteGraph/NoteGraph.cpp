#include "NoteGraph.h"
#include "../NoteColors.h"
#include "../../../Render/Draw/Draw.h"
#include "../../../Globals.h"

float NoteGraph::GetNoteAreaScreenHeight(Area screenArea) {
	return vScale * (screenArea.Height() - (screenArea.Height() / KEY_AMOUNT) * 2);
}

Vec NoteGraph::ToScreenPos(GraphPos graphPos, Area screenArea) {
	int relativeTime = graphPos.x - hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	float outY = -((graphPos.y - (KEY_AMOUNT_SUB1_F /2.f)) / KEY_AMOUNT_SUB1_F) * GetNoteAreaScreenHeight(screenArea);
return Vec(outX, outY) + screenArea.Center();
}

GraphPos NoteGraph::ToGraphPos(Vec screenPos, Area screenArea) {
	screenPos -= screenArea.Center();

	NoteTime x = hScroll + (NoteTime)((1000.f * screenPos.x) / hZoom);

	float height = GetNoteAreaScreenHeight(screenArea);

	// Didn't feel like doing algebra today so:
	// https://www.wolframalpha.com/input?i=x+%3D+-%28%28y+-+%28a+%2F2%29%29+%2F+a%29+*+v%2C+solve+for+y
	float y = (KEY_AMOUNT_SUB1_F * (height - 2 * screenPos.y)) / (2 * height);

	return { x, y };
}

void NoteGraph::CheckFixNoteOverlap(Note* note) {
	auto& slot = _noteSlots[note->key];
	for (auto itr = slot.begin(); itr != slot.end(); itr++) {
		Note* otherNote = *itr;
		if (note == otherNote)
			continue;

		if (
			note->time == otherNote->time // Direct collide
			|| (note->time < otherNote->time && note->time + note->duration > otherNote->time) // Our tail overlaps them
			) {

			itr--;
			RemoveNote(otherNote);
		} else {
			if (otherNote->time < note->time && otherNote->time + otherNote->duration > note->time) // Their tail overlaps us
				otherNote->duration = note->time - otherNote->time; // Clamp tail
		}
	}
}

void NoteGraph::MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap) {
	IASSERT(newY, KEY_AMOUNT);

	if (newY != note->key) {
		// Update slots if needed
		_noteSlots[note->key].erase(note);
		_noteSlots[newY].insert(note);
	}

	note->time = newX;
	note->key = newY;

	if (!ignoreOverlap)
		CheckFixNoteOverlap(note);
}

Note* NoteGraph::AddNote(Note note) {
	ASSERT(note.IsValid());

	Note* newNote = new Note(note);
	_notes.insert(newNote);
	_noteSlots[note.key].insert(newNote);

	// Update _furthestNoteEndTime
	NoteTime endTime = newNote->time + newNote->duration;
	_furthestNoteEndTime = MAX(_furthestNoteEndTime, endTime);

	CheckFixNoteOverlap(newNote);

	return newNote;
}

bool NoteGraph::RemoveNote(Note* note) {

	if (note == hoveredNote)
		hoveredNote = NULL;

	selectedNotes.erase(note);
	_noteSlots[note->key].erase(note);
	bool found = _notes.erase(note);
	if (found)
		delete note;
	return found;
}

void NoteGraph::ClearNotes() {
	for (Note* note : _notes) {
		delete note;
	}

	selectedNotes.clear();
	
	for (auto& slot : _noteSlots)
		slot.clear();
	
	_notes.clear();

	hoveredNote = NULL;
}

int NoteGraph::GetNoteCount() {
	return _notes.size();
}

void NoteGraph::UpdateWithInput(Area screenArea, SDL_Event& e) {
	bool isShiftDown = g_KeyboardState[SDL_SCANCODE_RSHIFT] || g_KeyboardState[SDL_SCANCODE_LSHIFT];
	bool isControlDown = g_KeyboardState[SDL_SCANCODE_RCTRL] || g_KeyboardState[SDL_SCANCODE_LCTRL];

	// Scroll graph
	if (e.type == SDL_MOUSEWHEEL) {
		hScroll += -e.wheel.y * (isShiftDown ? 5000 : 500);
		hScroll = CLAMP(g_NoteGraph.hScroll, 0, g_NoteGraph.GetFurthestNoteEndTime());
	}

	if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
		bool down = e.type == SDL_MOUSEBUTTONDOWN;
		int mouseButton = e.button.button;
		if (mouseButton == 1) {

		}
	}

	{ // Always update hovered note (this could change with mouse moving, scrolling, window resize, etc.)
		GraphPos graphMousePos = ToGraphPos(g_MousePos, screenArea);
		KeyInt graphMouseKey = roundf(graphMousePos.y);
		hoveredNote = NULL;
		for (Note* note : *this) {
			if (
				(graphMousePos.x >= note->time && graphMousePos.x < note->time + note->duration) // X is overlapping
				&& (graphMouseKey == note->key) // Y is overlapping
				) {


				// Prioritize later notes
				if (hoveredNote && note->time < hoveredNote->time)
					continue;

				hoveredNote = note;
			}
		}
	}
}

bool NoteGraph::TryMoveSelectedNotes(int amountX, int amountY) {
	// Check
	for (Note* note : selectedNotes) {
		if (note->time + amountX < 0)
			return false; // Horizontal limit reached

		int testNewKey = note->key + amountY;
		if (testNewKey < 0 || testNewKey >= KEY_AMOUNT)
			return false; // Vertical limit reached
	}

	// Move
	for (Note* note : selectedNotes)
		MoveNote(note, note->time + amountX, note->key + amountY, true);

	// Fix overlap after (otherwise we would "fix overlap" partway-through the movement)
	for (Note* note : selectedNotes)
		CheckFixNoteOverlap(note);

	return true;
}

void NoteGraph::Render(Area screenArea) {
	GraphPos graphMousePos = ToGraphPos(g_MousePos, screenArea);

	KeyInt graphMouseKey = roundf(graphMousePos.y);
	NoteTime graphMouseTime = graphMousePos.x;

	float noteSize = floorf(GetNoteAreaScreenHeight(screenArea) / KEY_AMOUNT);

	Draw::Rect(screenArea.min, screenArea.max, COL_BLACK);

	float minDrawX = MAX(screenArea.min.x, ToScreenPos({ 0,0 }, screenArea).x);

	// Horizontal slot lines
	for (int i = 0; i < KEY_AMOUNT; i++) {
		auto screenY = screenArea.min.y + ToScreenPos(GraphPos(0, i), screenArea).y;

		Draw::PixelPerfectLine(Vec(minDrawX, screenY), Vec(screenArea.max.x, screenY), Color(25, 25, 25));
	}

	// Vertical lines lines
	for (int i = 0; i < 100; i++) {
		float beat = i / 4.f;
		NoteTime t = beat * NOTETIME_PER_BEAT;
		bool isBeatLine = (beat == roundf(beat));
		bool isMeasureLine = isBeatLine && ((int)beat % timeSig.num) == 0;

		// More alpha = more important line
		int alpha = isMeasureLine ? 80 : (isBeatLine ? 45 : 15);

		auto screenX = screenArea.min.x + ToScreenPos(GraphPos(t, 0), screenArea).x;

		Draw::PixelPerfectLine(Vec(screenX, screenArea.min.y), Vec(screenX, screenArea.max.y),
			Color(255, 255, 255, alpha));
	}

	{ // Draw notes

		// We need to draw notes in order of time, this will contain note pointers but sorted
		vector<Note*> notesToDraw;

		for (Note* note : *this) {
			// TODO: Sub-optimal check requires function calls
			Vec screenPos = screenArea.min + ToScreenPos(GraphPos(note->time, note->key), screenArea);
			float tailEndX = screenArea.min.x + ToScreenPos(GraphPos(note->time + note->duration, 0), screenArea).x;

			if (screenPos.x > screenArea.max.x || tailEndX < screenArea.min.x)
				continue;

			notesToDraw.push_back(note);
		}

		// Lambda sorting by time
		std::sort(notesToDraw.begin(), notesToDraw.end(),
			[](const Note* a, const Note* b) -> bool {
				return a->time < b->time;
			}
		);

		float noteHeadSize_half = floorf(noteSize / 2);
		float noteHeadHollowSize_half = noteSize / 3;
		float noteTailGap = MAX(1.f, roundf(noteSize / 6.f));

		bool anySelected = !selectedNotes.empty();

		for (Note* note : notesToDraw) {
			bool selected = IsNoteSelected(note);

			Vec screenPos = screenArea.min + ToScreenPos(GraphPos(note->time, note->key), screenArea);
			float tailEndX = screenArea.min.x + ToScreenPos(GraphPos(note->time + note->duration, 0), screenArea).x;

			// Align pixel-perfect
			screenPos = screenPos.Rounded();

			Color col = NoteColors::GetKeyColor(note->key);

			Area headArea = { screenPos - noteHeadSize_half , screenPos + noteHeadSize_half };
			Area tailArea = { screenPos - Vec(0, noteTailGap), Vec(tailEndX, screenPos.y + noteTailGap) };
			if (selected) {
				// White outline around black border
				Draw::Rect(headArea.Expand(2), COL_WHITE);
				Draw::Rect(tailArea.Expand(2), COL_WHITE);

			} else {
				if (anySelected) {
					// Other notes are selected
					// Dim the color of this note to make it more clear it is NOT selected
					col = col.RatioBrighten(0.35f);
				}
			}

			// Brighten hovered notes
			if (hoveredNote == note)
				col = col.RatioBrighten(1.5f);

			// Black border for spacing
			Draw::Rect(headArea.Expand(1), COL_BLACK);
			Draw::Rect(tailArea.Expand(1), COL_BLACK);

			// Draw note body
			Draw::Rect(headArea, col);
			Draw::Rect(tailArea, col);

			// Hollow note head if black key
			if (note->IsBlackKey())
				Draw::Rect(screenPos - noteHeadHollowSize_half, screenPos + noteHeadHollowSize_half, COL_BLACK);
		}
	}

	if (currentMode == NoteGraph::MODE_RECTSELECT) {
		Vec start = ToScreenPos(modeInfo.startDragPos, screenArea);

		Area selectArea = { ToScreenPos(modeInfo.startDragPos, screenArea), g_MousePos };
		Draw::ORect(selectArea, COL_WHITE);
	}
}
