#include "NoteGraph.h"
#include "../NoteColors.h"
#include "../../../Render/Draw/Draw.h"
#include "../../../Globals.h"

float NoteGraph::GetNoteAreaScreenHeight(Area screenArea) {
	return vScale * (screenArea.GetHeight() - (screenArea.GetHeight() / KEY_AMOUNT) * 2);
}

Vec NoteGraph::ToScreenPos(GraphPos graphPos, Area screenArea) {
	int relativeTime = graphPos.x - hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	float outY = -((graphPos.y - (KEY_AMOUNT_SUB1_F /2.f)) / KEY_AMOUNT_SUB1_F) * GetNoteAreaScreenHeight(screenArea);
	return Vec(outX, outY) + screenArea.GetCenter();
}

GraphPos NoteGraph::ToGraphPos(Vec screenPos, Area screenArea) {
	screenPos -= screenArea.GetCenter();

	NoteTime x = hScroll + (NoteTime)((1000.f * screenPos.x) / hZoom);

	float height = GetNoteAreaScreenHeight(screenArea);

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

void NoteGraph::UpdateWithInput(Area screenArea, SDL_Event& e) {
	bool isShiftDown = g_KeyboardState[SDL_SCANCODE_RSHIFT] || g_KeyboardState[SDL_SCANCODE_LSHIFT];
	bool isControlDown = g_KeyboardState[SDL_SCANCODE_RCTRL] || g_KeyboardState[SDL_SCANCODE_LCTRL];

	// Scroll graph
	if (e.type == SDL_MOUSEWHEEL) {
		hScroll += -e.wheel.y * (isShiftDown ? 5000 : 500);
		hScroll = CLAMP(g_NoteGraph.hScroll, 0, g_NoteGraph.GetFurthestNoteEndTime());
		return;
	}

	if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
		int mouseButton = e.button.button;
		if (mouseButton == 1) {
			// ...
		}
	}
}

void NoteGraph::Draw(Area screenArea) {
	Vec relativeMousePos = g_MousePos - screenArea.min;
	GraphPos graphMousePos = ToGraphPos(relativeMousePos, screenArea);

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

		// Lambda sorting
		std::sort(notesToDraw.begin(), notesToDraw.end(),
			[](const Note* a, const Note* b) -> bool {
				return a->time < b->time;
			}
		);

		float noteHeadSize_half = floorf(noteSize / 2);
		float noteHeadHollowSize_half = noteSize / 3;
		float noteTailGap = MAX(1.f, roundf(noteSize / 6.f));

		bool anySelected = !selectedNotes.empty();

		bool hoveredNoteFound = false;

		for (Note* note : notesToDraw) {
			bool selected = IsNoteSelected(note);

			bool hovered =
				(graphMouseTime >= note->time && graphMouseTime < note->time + note->duration)
				&& (graphMouseKey == note->key)
				&& !hoveredNoteFound;

			// TODO: Move hovered note updating elsewhere
			if (hovered) {
				hoveredNoteFound = true;
				hoveredNote = note;
			}

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
			if (hovered) {
				col = col.RatioBrighten(1.5f);
			}

			// Black border for spacing
			Draw::Rect(headArea.Expand(1), COL_BLACK);
			Draw::Rect(tailArea.Expand(1), COL_BLACK);

			// Draw note body
			Draw::Rect(headArea, col);
			Draw::Rect(tailArea, col);

			// Hollow note head if black key
			if (note->IsBlackKey()) {
				Draw::Rect(screenPos - noteHeadHollowSize_half, screenPos + noteHeadHollowSize_half, COL_BLACK);
			}
		}
	}

	if (currentMode == NoteGraph::MODE_RECTSELECT) {
		Vec start = ToScreenPos(modeInfo.startDragPos, screenArea);

		Area selectArea = { ToScreenPos(modeInfo.startDragPos, screenArea), relativeMousePos };
		Draw::ORect(selectArea, COL_WHITE);
	}
}
