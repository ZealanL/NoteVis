#include "NoteGraph.h"
#include "../NoteColors.h"
#include "../../../Render/Draw/Draw.h"
#include "../../../Globals.h"

// NG_NOTIF but local
#define NOTIF(s, ...) this->logNotifs.Add(std::format(s, ##__VA_ARGS__))

bool NoteGraph::TryHandleSpecialKeyEvent(SDL_Keycode key, BYTE kbFlags) {

	// Move by interval
	if (!selectedNotes.empty() && key >= SDLK_2 && key <= SDLK_9) {
		// Number pressed, move selected notes
		bool duplicate = kbFlags & KBFLAG_CTRL;
		bool invertDir = kbFlags & KBFLAG_SHIFT;
		bool minor = kbFlags & KBFLAG_ALT;

		int intervalOffset = key - SDLK_1;
		int keyOffset = Intervals::GetIntervalKeyOffset(intervalOffset, minor, invertDir);

		// Check if this interval isn't different in maj/min (e.x. perfect 4th)
		// TODO: This is inefficient (add something in intervals?)
		bool minorMajorSame = keyOffset == Intervals::GetIntervalKeyOffset(intervalOffset, !minor, invertDir);

		if (duplicate) {
			vector<Note> notesToAdd;
			for (auto selected : selectedNotes)
				notesToAdd.push_back(*selected);

			for (auto note : notesToAdd)
				AddNote(note, true);
		}

		bool succeeded = TryMoveSelectedNotes(0, keyOffset);
		if (succeeded) {
			NOTIF("{} selected note(s) {} by a {}.",
				duplicate ? "Dupe-moved" : "Moved",
				invertDir ? "down" : "up",

				// Just say "octave" if it was an octave, otherwise say the interval
				(keyOffset % KEYS_PER_OCTAVE) ? ((minorMajorSame ? "" : (minor ? "minor " : "major ")) + FW::NumOrdinal(intervalOffset + 1)) : "octave"
			);
		}

		return true;
	}

	return false;
}

float NoteGraph::GetTopBarHeight(RenderContext* ctx) {
	// TODO: This is pretty lame and messy
	float height = MIN(ctx->fullNoteGraphScreenArea.Height() / 2.f, 32.f);
	return height;
}

Area NoteGraph::GetNoteAreaScreen(RenderContext* ctx) {
	Area noteArea = ctx->fullNoteGraphScreenArea;
	noteArea.min.y += GetTopBarHeight(ctx);
	return noteArea;
}

Vec NoteGraph::ToScreenPos(GraphPos graphPos, RenderContext* ctx) {
	int relativeTime = graphPos.x - hScroll;
	float outX = (relativeTime / 1000.f) * hZoom;

	auto noteAreaScreen = GetNoteAreaScreen(ctx);

	float outY = -((graphPos.y - (KEY_AMOUNT_SUB1_F / 2.f)) / KEY_AMOUNT_SUB1_F) * (noteAreaScreen.Height() * vScale);
	return Vec(outX, outY) + noteAreaScreen.Center();
}

GraphPos NoteGraph::ToGraphPos(Vec screenPos, RenderContext* ctx) {

	auto noteAreaScreen = GetNoteAreaScreen(ctx);

	screenPos -= noteAreaScreen.Center();

	NoteTime x = hScroll + (NoteTime)((1000.f * screenPos.x) / hZoom);

	float height = noteAreaScreen.Height() * vScale;

	// Didn't feel like doing algebra today so:
	// https://www.wolframalpha.com/input?i=x+%3D+-%28%28y+-+%28a+%2F2%29%29+%2F+a%29+*+v%2C+solve+for+y
	float y = (KEY_AMOUNT_SUB1_F * (height - 2 * screenPos.y)) / (2 * height);

	return { x, y };
}

void NoteGraph::CheckFixNoteOverlap(Note* note) {
	auto& slot = _noteSlots[note->key];
	vector<Note*> notesToRemove;
	for (Note* otherNote : slot) {
		if (note == otherNote)
			continue;

		if (
			note->time == otherNote->time // Direct collide
			|| (note->time < otherNote->time && note->time + note->duration > otherNote->time) // Our tail overlaps them
			) {

			notesToRemove.push_back(otherNote);
		} else {
			if (otherNote->time < note->time && otherNote->time + otherNote->duration > note->time) // Their tail overlaps us
				otherNote->duration = note->time - otherNote->time; // Clamp tail
		}
	}

	for (Note* toRemove : notesToRemove)
		RemoveNote(toRemove);
}

void NoteGraph::MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap) {
	ASSERT(note->IsValid());
	IASSERT(newY, KEY_AMOUNT);

	if (newY != note->key) {
		// Update slots if needed
		_noteSlots[note->key].erase(note);
		_noteSlots[newY].insert(note);
	}

	note->time = newX;
	note->key = newY;

	DLOG("Moved note to {}", newY);

	if (!ignoreOverlap)
		CheckFixNoteOverlap(note);
}

Note* NoteGraph::AddNote(Note note, bool ignoreOverlap) {
	ASSERT(note.IsValid());

	Note* newNote = new Note(note);
	_notes.insert(newNote);
	_noteSlots[note.key].insert(newNote);

	// Update _furthestNoteEndTime
	NoteTime endTime = newNote->time + newNote->duration;
	_furthestNoteEndTime = MAX(_furthestNoteEndTime, endTime);

	if (!ignoreOverlap)
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

void NoteGraph::ClearEverything() {
	ClearNotes();
	NOTIF("Cleared everything.");
}

void NoteGraph::UpdateWithInput(SDL_Event& e, RenderContext* ctx) {
	bool isLMouseDown = g_MouseState[SDL_BUTTON_LEFT];
	bool isShiftDown =		g_KeyboardState[SDL_SCANCODE_RSHIFT]	|| g_KeyboardState[SDL_SCANCODE_LSHIFT];
	bool isControlDown =	g_KeyboardState[SDL_SCANCODE_RCTRL]		|| g_KeyboardState[SDL_SCANCODE_LCTRL];
	bool isAltDown =		g_KeyboardState[SDL_SCANCODE_RALT]		|| g_KeyboardState[SDL_SCANCODE_LALT];

	GraphPos mouseGraphPos = ToGraphPos(g_MousePos, ctx);
	KeyInt mouseGraphKey = roundf(mouseGraphPos.y);

	static GraphPos lastMouseGraphPos;
	bool mouseMoved = mouseGraphPos != lastMouseGraphPos;
	lastMouseGraphPos = mouseGraphPos;

	bool isDragging = isLMouseDown && mouseMoved;

	{ // Update hoveredNote
		if (currentMode == MODE_IDLE) {
			if (mouseMoved) {
				hoveredNote = NULL;
				for (Note* note : *this) {

					int screenNoteStartTime = ToScreenPos({ note->time, (float)note->key }, ctx).x;
					int screenNoteEndTime = 
						MAX(
							ToScreenPos({ note->time + note->duration, (float)note->key }, ctx).x, 
							screenNoteStartTime + (GetNoteBaseHeadSizeScreen(ctx) * 2)
						);
					
					if (
						(g_MousePos.x >= screenNoteStartTime && g_MousePos.x < screenNoteEndTime) // X is overlapping
						&& (mouseGraphKey == note->key) // Y is overlapping
						) {

						// Prioritize later notes
						if (hoveredNote && note->time < hoveredNote->time)
							continue;

						hoveredNote = note;
					}
				}
			}
		} else {
			hoveredNote = NULL; // Note hovering doesn't exist in other modes
		}
	}

	// Scroll graph
	if (e.type == SDL_MOUSEWHEEL) {
		int scrollDelta = e.wheel.y;

		// scroll = move graph horizontally
		//	+ control = zoom in/out horizontally
		// scroll + alt = move graph vertically
		//	+ control + alt = zoom in/out vertically

		// if shift is down, scroll/move faster

		bool horizontal = !isAltDown;

		if (horizontal) {
			if (isControlDown) {
				// Horizontal zoom
				hZoom *= 1 + (scrollDelta / (isShiftDown ? 1.f : 10.f));
				hZoom = CLAMP(hZoom, 20, 2000);
			} else {
				// Horizontal scroll
				hScroll += -scrollDelta * (isShiftDown ? 5000 : 500) / (hZoom / 100);
				hScroll = CLAMP(hScroll, 0, GetGraphEndTime());
			}
		} else {
			// TODO: Vertical scrolling maybe?
			/*
			if (isControlDown) {
				// Vertical zoom
				vScale *= 1 + (scrollDelta / (isShiftDown ? 1.f : 10.f));
				vScale = CLAMP(vScale, 1, 20);
			} else {
				// Vertical scroll
				
			}
			*/
		}
	}

	// Mouse click
	if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
		bool down = e.type == SDL_MOUSEBUTTONDOWN;

		int mouseButton = e.button.button;
		if (mouseButton == SDL_BUTTON_LEFT) {
			// Left click

			if (down) {
				// Reset mode drag info
				state.dragInfo.selectedNoteLastMouseDown = false;
				state.dragInfo.startDragPos = ToGraphPos(g_MousePos, ctx);
				state.dragInfo.startDragMousePos = g_MousePos;
				state.dragInfo.startDragSelectedNote = NULL;
			}

			switch (currentMode) {
			case MODE_IDLE:
				if (down) {
					if (!isShiftDown) // If not holding shift, deselect everything
						selectedNotes.clear();

					if (hoveredNote && !IsNoteSelected(hoveredNote)) {
						selectedNotes.insert(hoveredNote);
						state.dragInfo.selectedNoteLastMouseDown = true;
						state.dragInfo.startDragSelectedNote = hoveredNote;
					}

				} else {
					if (hoveredNote) {
						if (isShiftDown && IsNoteSelected(hoveredNote) && !state.dragInfo.selectedNoteLastMouseDown)
							selectedNotes.erase(hoveredNote); // Deselect when holding shift
					}

					break;
				}

			case MODE_RECTSELECT:
			case MODE_DRAGNOTES: {
				// Stop selecting/dragging
				currentMode = MODE_IDLE;

				for (Note* selectedNote : selectedNotes)
					CheckFixNoteOverlap(selectedNote);

				break;
			}
			}
		} else if (mouseButton == SDL_BUTTON_RIGHT) {
			// Right click

			if (down) {
				// Move playhead
				// TODO: Global framework snapping function
				state.playInfo.startTime = CLAMP(mouseGraphPos.x, 0, GetGraphEndTime());
				state.playInfo.startTime += snappingTime / 2;
				state.playInfo.startTime -= state.playInfo.startTime % snappingTime;

			}
		}
	}

	constexpr float MIN_MOUSE_DRAG_DIST_PX = 3.f;

	if (isDragging) {
		if (isControlDown) {
			// Start rect selection
			currentMode = MODE_RECTSELECT;
		} else if (state.dragInfo.selectedNoteLastMouseDown
			&& (g_MousePos.Distance(state.dragInfo.startDragMousePos) >= MIN_MOUSE_DRAG_DIST_PX)) {
			// Standard selection
			// Also possible drag
			currentMode = MODE_DRAGNOTES;
		}
	}

	// Update selected notes in rect
	if (currentMode == MODE_RECTSELECT) {
		GraphPos a = state.dragInfo.startDragPos, b = ToGraphPos(g_MousePos, ctx);

		NoteTime startTime = MIN(a.x, b.x), endTime = MAX(a.x, b.x);
		KeyInt startKey = roundf(MIN(a.y, b.y)), endKey = roundf(MAX(a.y, b.y));

		if (!isShiftDown)
			selectedNotes.clear();

		for (Note* note : *this) {
			if (note->time + note->duration >= startTime && note->time <= endTime)
				if (note->key >= startKey && note->key <= endKey)
					selectedNotes.insert(note);
		}
	}

	if (currentMode == MODE_DRAGNOTES) {
		if (!selectedNotes.empty()) {
			int timeDelta = mouseGraphPos.x - state.dragInfo.startDragPos.x;
			int keyDelta = roundf(mouseGraphPos.y - state.dragInfo.startDragPos.y);

			Note* first = *selectedNotes.begin();
			Note* earliestNote = first;
			NoteTime minTime = first->time;
			KeyInt minKey = first->key, maxKey = first->key;
			for (Note* note : selectedNotes) {
				minKey = MIN(note->key, minKey);
				maxKey = MAX(note->key, maxKey);

				if (note->time < minTime) {
					earliestNote = note;
					minTime = note->time;
				}
			}

			// Limit move
			keyDelta = CLAMP(keyDelta, -minKey, KEY_AMOUNT - maxKey - 1);
			timeDelta = CLAMP(timeDelta, -minTime, timeDelta);

			if (snappingTime > 1) {
				// This will be the positional basis for our snapping
				Note* baseNoteForSnap = state.dragInfo.startDragSelectedNote;

				// If dragInfo.startDragSelectedNote wasn't set, just use the earliest selected note
				if (!baseNoteForSnap)
					baseNoteForSnap = earliestNote;

				NoteTime unsnappedMoveTime = baseNoteForSnap->time + timeDelta;
				timeDelta -= ((unsnappedMoveTime + (snappingTime / 2)) % snappingTime) - (snappingTime / 2);
			}

			if (TryMoveSelectedNotes(timeDelta, keyDelta, true)) {

				state.dragInfo.startDragPos.x += timeDelta;
				state.dragInfo.startDragPos.y += keyDelta;
			}
		}
	}
}

bool NoteGraph::TryMoveSelectedNotes(int amountX, int amountY, bool ignoreOverlap) {
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
	if (!ignoreOverlap)
		for (Note* note : selectedNotes)
			CheckFixNoteOverlap(note);

	return true;
}

void NoteGraph::Serialize(ByteDataStream& bytesOut) {
	bytesOut.reserve(GetNoteCount() * sizeof(Note));

	for (Note* note : _notes) {
		bytesOut.WriteAsBytes(note->key);
		bytesOut.WriteAsBytes(note->time);
		bytesOut.WriteAsBytes(note->duration);
		bytesOut.WriteAsBytes(note->velocity);

		bytesOut.WriteAsBytes(IsNoteSelected(note));
	}

	DLOG("NoteGraph::Serialize(): Wrote {} notes (bytesOut size: {})", _notes.size(), bytesOut.size());
}

void NoteGraph::Deserialize(ByteDataStream::ReadIterator& bytesIn) {
	int notesRead;
	for (notesRead = 0; bytesIn.BytesLeft(); notesRead++) {
		Note newNote;
		bytesIn.Read(&newNote.key);
		bytesIn.Read(&newNote.time);
		bytesIn.Read(&newNote.duration);
		bytesIn.Read(&newNote.velocity);
		Note* note = AddNote(newNote);

		bool selected;
		bytesIn.Read(&selected);
		if (selected)
			selectedNotes.insert(note);
	}

	DLOG("NoteGraph::Deserialize(): Read {} notes", notesRead);
}

int NoteGraph::GetNoteBaseHeadSizeScreen(RenderContext* ctx) {
	return floorf((GetNoteAreaScreen(ctx).Height() * vScale) / KEY_AMOUNT);
}

void NoteGraph::RenderNotes(RenderContext* ctx) {
	Area noteAreaScreen = GetNoteAreaScreen(ctx);
	Vec screenMin = noteAreaScreen.min, screenMax = noteAreaScreen.max;

	GraphPos graphMousePos = ToGraphPos(g_MousePos, ctx);

	KeyInt graphMouseKey = roundf(graphMousePos.y);
	NoteTime graphMouseTime = graphMousePos.x;

	NoteTime graphViewStartTime = ToGraphPos(noteAreaScreen.min, ctx).x;
	NoteTime graphViewEndTime = ToGraphPos(noteAreaScreen.max, ctx).x;

	// Round start time down to nearest beat
	graphViewStartTime = (graphViewStartTime / NOTETIME_PER_BEAT) * NOTETIME_PER_BEAT;

	float noteSize = GetNoteBaseHeadSizeScreen(ctx);

	float minDrawX = MAX(noteAreaScreen.min.x, ToScreenPos({ 0,0 }, ctx).x);

	// Horizontal slot lines
	for (int i = 0; i < KEY_AMOUNT; i++) {
		auto screenY = ToScreenPos(GraphPos(0, i), ctx).y;

		Draw::PixelPerfectLine(Vec(minDrawX, screenY), Vec(noteAreaScreen.max.x, screenY), Color(25, 25, 25));
	}

	// Vertical lines lines
	for (int i = MAX(0, graphViewStartTime); i <= graphViewEndTime; i += (NOTETIME_PER_BEAT / 4)) {
		bool isBeatLine = i % NOTETIME_PER_BEAT == 0;
		int beat = i / NOTETIME_PER_BEAT;
		bool isMeasureLine = isBeatLine && beat % timeSig.beatCount == 0;
		int measure = beat / timeSig.beatCount;

		// More alpha = more important line
		int alpha = isMeasureLine ? 80 : (isBeatLine ? 45 : 15);

		auto screenX = ToScreenPos(GraphPos(i, 0), ctx).x;

		Color color = Color(255, 255, 255, alpha);

		Draw::Line(Vec(screenX, screenMin.y), Vec(screenX, screenMax.y), color);

		// Measure numbers
		if (isMeasureLine)
			Draw::Text(std::to_string(measure + 1), Vec(screenX, screenMin.y), color, { -0.5f, -0.25f });
	}

	{ // Draw notes

// We need to draw notes in order of time, this will contain note pointers but sorted
		vector<Note*> notesToDraw;

		// TODO: Sub-optimal check requires function calls
		for (Note* note : *this) {
			Vec screenPos = ToScreenPos(GraphPos(note->time, note->key), ctx);
			float tailEndX = ToScreenPos(GraphPos(note->time + note->duration, 0), ctx).x;

			if (screenPos.x > screenMax.x || tailEndX < screenMin.x)
				continue;

			notesToDraw.push_back(note);
		}

		// Sorting by time
		std::sort(notesToDraw.begin(), notesToDraw.end(),
			[](const Note* a, const Note* b) -> bool {
				return a->time < b->time;
			}
		);

		float noteHeadSize_half = floorf(noteSize / 2);
		float noteHeadHollowSize_half = noteSize / 3;
		float noteTailGap = MAX(1.f, roundf(noteSize / 6.f));

		bool dimNonSelected;
		switch (currentMode) {
		case MODE_PLAY:
			dimNonSelected = false;
			break;
		case MODE_RECTSELECT:
			dimNonSelected = true;
			break;
		default:
			dimNonSelected = !selectedNotes.empty();
		}

		for (Note* note : notesToDraw) {
			bool selected = IsNoteSelected(note);

			Vec screenPos = ToScreenPos(GraphPos(note->time, note->key), ctx);
			float tailEndX = ToScreenPos(GraphPos(note->time + note->duration, 0), ctx).x;

			// Align pixel-perfect
			screenPos = screenPos.Rounded();

			Color col = NoteColors::GetKeyColor(note->key);
			Color tailCol = col;

			float velocityRatio = powf(note->velocity / 255.f, 0.5f);
			float headScale = 0.6f + velocityRatio;

			col.a = 155 + (velocityRatio * 100);
			tailCol.a = 55 + (velocityRatio * 200);

			Vec headCenterPos = screenPos + Vec(noteHeadSize_half * headScale, 0);

			Area headArea = { headCenterPos - noteHeadSize_half * headScale, headCenterPos + noteHeadSize_half * headScale };
			Area tailArea = { headCenterPos - Vec(0, noteTailGap), Vec(tailEndX, headCenterPos.y + noteTailGap) };
			tailArea.min.x = MAX(tailArea.min.x, headArea.max.x);

			if (selected) {
				// Selected notes are outlined with white
				Draw::Rect(headArea.Expand(2), COL_WHITE);
				Draw::Rect(tailArea.Expand(2), COL_WHITE);

			} else {
				if (dimNonSelected)
					col = col.RatioBrighten(0.35f);
			}

			// Brighten hovered notes
			if (hoveredNote == note)
				col = col.RatioBrighten(1.5f);

			// Black border for spacing
			Draw::Rect(headArea.Expand(1), COL_BLACK);
			Draw::Rect(tailArea.Expand(1), COL_BLACK);

			// Draw note body
			Draw::Rect(headArea, col);

			// In some cases, the tail might end before it starts, which means it's too short to be visible
			if (tailArea.min.x < tailArea.max.x)
				Draw::Rect(tailArea, tailCol);

			// Really loud notes begin to have a halo around their head
			constexpr int HALO_THRESHOLD = 100;
			int haloWidth = (MAX(0, note->velocity - HALO_THRESHOLD) / (255.f - HALO_THRESHOLD)) * noteHeadSize_half;
			if (haloWidth > 0) {
				Draw::Rect(headArea.Expand(haloWidth), Color(col.r, col.g, col.b, 50));
			}

			// Hollow note head if black key
			if (note->IsBlackKey())
				Draw::Rect(headCenterPos - noteHeadHollowSize_half, headCenterPos + noteHeadHollowSize_half, COL_BLACK);
		}
	}

	{ // Draw playhead
		float screenX = ToScreenPos(GraphPos(state.playInfo.startTime, 0), ctx).x;

		Vec topPoint = Vec(screenX, screenMin.y);
		float height = MIN(GetTopBarHeight(ctx) / 2.f, 8.f);

		// Playhead line
		Draw::PixelPerfectLine(topPoint, Vec(screenX, screenMax.y), COL_WHITE);
		Draw::PixelPerfectLine(Vec(screenX-1, screenMin.y), Vec(screenX-1, screenMax.y), COL_BLACK);
		Draw::PixelPerfectLine(Vec(screenX+1, screenMin.y), Vec(screenX+1, screenMax.y), COL_BLACK);

		vector<Vec> playheadPoints = {
			topPoint,
			topPoint + Vec(-height,	-height),
			topPoint + Vec(height,	-height),
		};

		Draw::ConvexPoly(playheadPoints, COL_WHITE);
	}

	if (currentMode == NoteGraph::MODE_RECTSELECT) {
		Vec start = ToScreenPos(state.dragInfo.startDragPos, ctx);

		Area selectArea = { ToScreenPos(state.dragInfo.startDragPos, ctx), g_MousePos };
		Draw::ORect(selectArea, COL_WHITE);
	}
}

void NoteGraph::Render(RenderContext* ctx) {
	Draw::Rect(ctx->fullNoteGraphScreenArea, COL_BLACK);

	RenderNotes(ctx);

	float logNotifWidth = MIN(ctx->fullNoteGraphScreenArea.Width() / 2.f, 400.f);
	logNotifs.DrawAndUpdate(ctx->fullNoteGraphScreenArea.TopRight() - Vec(logNotifWidth + 4, 4), logNotifWidth);
}
