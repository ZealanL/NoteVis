#include "NoteGraph.h"
#include "../NoteColors.h"
#include "../../../Render/Draw/Draw.h"
#include "../../../Globals.h"

// NG_NOTIF but local
#define NOTIF(s, ...) this->logNotifs.Add(std::format(s, ##__VA_ARGS__))

bool NoteGraph::TryHandleSpecialKeyEvent(SDL_Keycode key, BYTE kbFlags) {

	// Move by interval
	if (!noteCache.selected.empty() && key >= SDLK_2 && key <= SDLK_9) {
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
			for (auto selected : noteCache.selected)
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
	auto& slot = noteCache.slots[note->key];
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
			if (otherNote->time < note->time && otherNote->time + otherNote->duration > note->time) {// Their tail overlaps us
				otherNote->duration = note->time - otherNote->time; // Clamp tail
				noteCache.OnNoteChanged(otherNote, false);
			}
		}
	}

	for (Note* toRemove : notesToRemove)
		RemoveNote(toRemove);
}

void NoteGraph::MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap) {
	ASSERT(note->IsValid());
	IASSERT(newY, KEY_AMOUNT);

	note->time = newX;
	note->key = newY;

	noteCache.OnNoteChanged(note, newY != note->key);

	if (!ignoreOverlap)
		CheckFixNoteOverlap(note);
}

void NoteGraph::TogglePlay() {
	if (currentMode == MODE_PLAY) {
		currentMode = MODE_IDLE;
		g_MIDIPlayer.StopAll();
	} else {
		currentMode = MODE_PLAY;
		state.playInfo.playStartTime = CURRENT_TIME;
		DLOG("Playing from t={}", state.playInfo.startGraphTime);
	}
}

Note* NoteGraph::AddNote(Note note, bool ignoreOverlap) {
	Note* newNote = noteCache.AddNote(note);

	if (!ignoreOverlap)
		CheckFixNoteOverlap(newNote);

	return newNote;
}

bool NoteGraph::RemoveNote(Note* note) {
	if (note == hoveredNote)
		hoveredNote = NULL;

	return noteCache.RemoveNote(note);
}

void NoteGraph::ClearNotes() {
	noteCache.Reset();
	hoveredNote = NULL;
}

int NoteGraph::GetNoteCount() {
	return noteCache.notes.size();
}

void NoteGraph::ClearEverything(bool notify) {
	ClearNotes();

	if (notify)
		NOTIF("Cleared everything.");
}

void NoteGraph::UpdateWithInput(SDL_Event& e, RenderContext* ctx) {
	bool shouldUpdateHistory = false;

	bool isLMouseDown = g_MouseState[SDL_BUTTON_LEFT];
	bool isShiftDown = g_KeyboardState[SDL_SCANCODE_RSHIFT] || g_KeyboardState[SDL_SCANCODE_LSHIFT];
	bool isControlDown = g_KeyboardState[SDL_SCANCODE_RCTRL] || g_KeyboardState[SDL_SCANCODE_LCTRL];
	bool isAltDown = g_KeyboardState[SDL_SCANCODE_RALT] || g_KeyboardState[SDL_SCANCODE_LALT];

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

	// Pick between idle or notevel adjust depending on keybinds
	if (currentMode == MODE_IDLE || currentMode == MODE_ADJUSTNOTEVEL)
		currentMode = g_Actions.ha_ChangeNoteVels->IsActive() ? MODE_ADJUSTNOTEVEL : MODE_IDLE;
		

	// Scroll graph
	if (e.type == SDL_MOUSEWHEEL) {
		shouldUpdateHistory = true;
		int scrollDelta = e.wheel.y;

		if (currentMode == MODE_ADJUSTNOTEVEL) {

			// Increase/decrease selected note velocities
			for (auto note : noteCache.selected) {
				note->velocity = CLAMP(
					note->velocity + scrollDelta
					* (isShiftDown ? 50 : 10), // Shift to change vel 5x faster
					1, 255);
				noteCache.OnNoteChanged(note, false);
			}

		} else {

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
	}

	// Mouse click
	if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
		shouldUpdateHistory = true;
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
						noteCache.DeselectAll();

					if (hoveredNote && !IsNoteSelected(hoveredNote)) {
						noteCache.SetSelected(hoveredNote, true);
						state.dragInfo.selectedNoteLastMouseDown = true;
					}

					state.dragInfo.startDragSelectedNote = hoveredNote;

				} else {
					if (hoveredNote) {
						if (isShiftDown && IsNoteSelected(hoveredNote) && !state.dragInfo.selectedNoteLastMouseDown)
							noteCache.SetSelected(hoveredNote, false); // Deselect when holding shift
					}

					break;
				}

			case MODE_RECTSELECT:
			case MODE_DRAGNOTES:
			case MODE_DRAGNOTELENGTHS: {
				// Stop selecting/dragging
				currentMode = MODE_IDLE;

				for (Note* selectedNote : noteCache.selected)
					CheckFixNoteOverlap(selectedNote);

				break;
			}
			}
		} else if (mouseButton == SDL_BUTTON_RIGHT) {
			// Right click

			if (down) {

				// Move playhead
				state.playInfo.startGraphTime = CLAMP(mouseGraphPos.x, 0, GetGraphEndTime());
				state.playInfo.startGraphTime = ISNAP(state.playInfo.startGraphTime, snappingTime);
				state.playInfo.curGraphTime = state.playInfo.startGraphTime;

				if (currentMode == MODE_PLAY) {
					// Prevent playing notes inbetween
					state.playInfo.playStartTime = CURRENT_TIME;

					// Prevent notes currently being played from getting stuck on
					g_MIDIPlayer.StopAll();
				}
			}
		}
	}

	constexpr float MIN_MOUSE_DRAG_DIST_PX = 3.f;

	if (isDragging) {
		shouldUpdateHistory = true;
		if (state.dragInfo.startDragSelectedNote) {
			if (g_MousePos.Distance(state.dragInfo.startDragMousePos) >= MIN_MOUSE_DRAG_DIST_PX) {
				// Standard selection
				// Also possible drag
				currentMode = isAltDown ? MODE_DRAGNOTELENGTHS : MODE_DRAGNOTES;
			}
		} else {
			// Started dragging in a blank area, start rect selection
			currentMode = MODE_RECTSELECT;
		}
	}

	// Update selected notes in rect
	if (currentMode == MODE_RECTSELECT) {
		shouldUpdateHistory = true;
		GraphPos a = state.dragInfo.startDragPos, b = ToGraphPos(g_MousePos, ctx);

		NoteTime startTime = MIN(a.x, b.x), endTime = MAX(a.x, b.x);
		KeyInt startKey = roundf(MIN(a.y, b.y)), endKey = roundf(MAX(a.y, b.y));

		if (!isShiftDown)
			noteCache.DeselectAll();

		for (Note* note : *this) {
			if (note->time + note->duration >= startTime && note->time <= endTime) {
				if (note->key >= startKey && note->key <= endKey) {
					noteCache.SetSelected(note, true);
				}
			}
		}
	}

	Note* dragBaseNote = state.dragInfo.startDragSelectedNote;
	if (currentMode == MODE_DRAGNOTES) {
		shouldUpdateHistory = true;
		if (!noteCache.selected.empty() && dragBaseNote) {
			int timeDelta = mouseGraphPos.x - state.dragInfo.startDragPos.x;
			int keyDelta = roundf(mouseGraphPos.y - state.dragInfo.startDragPos.y);

			Note* first = *noteCache.selected.begin();
			NoteTime minTime = first->time;
			KeyInt minKey = first->key, maxKey = first->key;
			for (Note* note : noteCache.selected) {
				minKey = MIN(note->key, minKey);
				maxKey = MAX(note->key, maxKey);
			}

			// Limit move
			keyDelta = CLAMP(keyDelta, -minKey, KEY_AMOUNT - maxKey - 1);
			timeDelta = CLAMP(timeDelta, -minTime, timeDelta);

			if (snappingTime > 1) {
				// This will be the positional basis for our snapping
				timeDelta = ISNAP(dragBaseNote->time + timeDelta, snappingTime) - dragBaseNote->time;
			}

			if (TryMoveSelectedNotes(timeDelta, keyDelta, true)) {
				state.dragInfo.startDragPos.x += timeDelta;
				state.dragInfo.startDragPos.y += keyDelta;
			}
		}
	} else if (currentMode == MODE_DRAGNOTELENGTHS) {
		shouldUpdateHistory = true;
		if (!noteCache.selected.empty() && dragBaseNote) {
			// Drag-moving selected notes' lengths

			NoteTime minDuration = MAX(1, snappingTime);
			NoteTime dragTimeDelta = ISNAP(mouseGraphPos.x - (dragBaseNote->time + dragBaseNote->duration), snappingTime);

			for (auto note : noteCache.selected) {
				note->duration = MAX(note->duration + dragTimeDelta, minDuration);
				noteCache.OnNoteChanged(note, false);
			}
		}
	}

	if (shouldUpdateHistory)
		g_History.Update();
}

bool NoteGraph::TryMoveSelectedNotes(int amountX, int amountY, bool ignoreOverlap) {
	// Check
	for (Note* note : noteCache.selected) {
		if (note->time + amountX < 0)
			return false; // Horizontal limit reached

		int testNewKey = note->key + amountY;
		if (testNewKey < 0 || testNewKey >= KEY_AMOUNT)
			return false; // Vertical limit reached
	}

	// Move
	for (Note* note : noteCache.selected) {
		MoveNote(note, note->time + amountX, note->key + amountY, true);
	}


	// Fix overlap after (otherwise we would "fix overlap" partway-through the movement)
	if (!ignoreOverlap)
		for (Note* note : noteCache.selected)
			CheckFixNoteOverlap(note);

	return true;
}

void NoteGraph::Serialize(ByteDataStream& bytesOut) {
	bytesOut.reserve(GetNoteCount() * sizeof(Note));

	for (Note* note : noteCache.sortedByStartTime) {
		ByteDataStream byteData;
		bytesOut.WriteAsBytes(note->key);
		bytesOut.WriteAsBytes(note->time);
		bytesOut.WriteAsBytes(note->duration);
		bytesOut.WriteAsBytes(note->velocity);
		bytesOut.WriteAsBytes(IsNoteSelected(note));
	}
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

		bool selected; bytesIn.Read(&selected);
		if (selected)
			noteCache.SetSelected(note, true);
	}
}

int NoteGraph::GetNoteBaseHeadSizeScreen(RenderContext* ctx) {
	constexpr float NOTEHEAD_SIZE_SCALE = 1.25f; // Slightly bigger than the slot
	return floorf((GetNoteAreaScreen(ctx).Height() * vScale) / KEY_AMOUNT * NOTEHEAD_SIZE_SCALE);
}

void NoteGraph::UpdatePlayOnRender() {

	// TODO: This is lazy, thread-unsafe, and very not-ideal
	static vector<KeyInt> keysToOffNextFrame;

	double timeSpentPlaying = CURRENT_TIME - state.playInfo.playStartTime;
	NoteTime nextCurTime = state.playInfo.startGraphTime + timeSpentPlaying * NOTETIME_PER_BEAT * 2;

	// Backup
	auto keysToOffThisFrame = keysToOffNextFrame;

	bool playedKeys[KEY_AMOUNT] {}; // Make sure we don't turn on and off a key in the same frame
	for (auto note : noteCache.sortedByStartTime) {
		ASSERT(note->IsValid());

		bool playedThisNote = false;

		if (note->time >= state.playInfo.curGraphTime 
			&& note->time < nextCurTime) {
			
			// We passed a note start, play the note
			g_MIDIPlayer.NoteOn(note->key, note->velocity);
			playedKeys[note->key] = true;
			playedThisNote = true;
		}

		if (!playedKeys[note->key]) {
			if (note->time + note->duration >= state.playInfo.curGraphTime
				&& note->time + note->duration < nextCurTime) {
				// We passed a note end, stop the note
				if (playedThisNote) {
					// We can't stop it on the same tick it starts, so wait till next frame
					keysToOffNextFrame.push_back(note->key);
				} else {
					g_MIDIPlayer.NoteOff(note->key);
				}
			}
		}
	}

	// Reset keys from last frame
	for (KeyInt key : keysToOffThisFrame) {
		if (!playedKeys)
			g_MIDIPlayer.NoteOff(key);
	}
	keysToOffNextFrame.clear();

	state.playInfo.curGraphTime = nextCurTime;
	hScroll = state.playInfo.curGraphTime;
}

void NoteGraph::RenderNotes(RenderContext* ctx) {

	if (currentMode == MODE_PLAY)
		UpdatePlayOnRender();

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

		// Determine visible notes
		// TODO: Sub-optimal to parse all notes each frame
		for (Note* note : *this) {
			if (note->time > graphViewEndTime || note->time + note->duration < graphViewStartTime)
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
		float noteHeadHollowSize_half = roundf(noteSize / 3);
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
			dimNonSelected = !noteCache.selected.empty();
		}

		for (Note* note : notesToDraw) {
			bool isSelected = IsNoteSelected(note);
			bool isBeingPlayed = currentMode == MODE_PLAY &&
				(note->time <= state.playInfo.curGraphTime 
					&& note->time + note->duration >= state.playInfo.curGraphTime);

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

			if (isBeingPlayed) {
				headScale *= 1.5f;
			}

			// Keep things pixel-perfect
			headCenterPos = headCenterPos.Rounded();

			Area headArea = { headCenterPos - noteHeadSize_half * headScale, headCenterPos + noteHeadSize_half * headScale };
			Area tailArea = { headCenterPos - Vec(0, noteTailGap), Vec(tailEndX, headCenterPos.y + noteTailGap) };
			tailArea.min.x = MAX(tailArea.min.x, headArea.max.x);

			if (isSelected) {
				// Selected notes are outlined with white on the outside
				Draw::Rect(headArea.Expand(2), COL_WHITE);

				if (currentMode == MODE_ADJUSTNOTEVEL) {
					// Show velocity below head
					int textAlpha = 50 + velocityRatio * 155;

					// Percentage of velocity from 1-100 (no percent sign shown)
					int percentage = MAX((int)(note->velocity / (255.f/100.f)), 1);

					Draw::Text(std::to_string(percentage), headArea.Bottom(), Color(255, 255, 255, textAlpha), Vec(0.5, -0.5f), textAlpha);
				} else {
					if (currentMode == MODE_DRAGNOTELENGTHS) {
						// Only outline end of tail
						Draw::Rect(tailArea.Move(Vec(2, 0)).Expand(1), COL_WHITE);
					} else {
						// Outline entire tail
						Draw::Rect(tailArea.Expand(2), COL_WHITE);
					}
				}

			} else {
				if (dimNonSelected) {
					col = col.RatioBrighten(0.35f);
				}
			}

			// Brighten hovered notes
			if (hoveredNote == note)
				col = col.RatioBrighten(1.5f);

			// Black border for spacing
			Draw::Rect(headArea.Expand(1), isBeingPlayed ? COL_WHITE : COL_BLACK);
			Draw::Rect(tailArea.Expand(1), isBeingPlayed ? COL_WHITE : COL_BLACK);

			// Draw note body
			Draw::Rect(headArea, col);

			// In some cases, the tail might end before it starts, which means it's too short to be visible
			if (tailArea.min.x < tailArea.max.x)
				Draw::Rect(tailArea, tailCol);

			// Really loud notes begin to have a halo/glow around their head
			constexpr int HALO_THRESHOLD = 100;
			int haloWidth = (MAX(0, note->velocity - HALO_THRESHOLD) / (255.f - HALO_THRESHOLD)) * noteHeadSize_half * 0.75f;
			if (haloWidth > 0) {
				Draw::Rect(headArea.Expand(haloWidth), Color(col.r, col.g, col.b, 50));
			}

			// Hollow note head if black key
			if (note->IsBlackKey()) {
				int holeSize = noteHeadHollowSize_half * headScale;
				Draw::Rect(headCenterPos - holeSize, headCenterPos + holeSize, COL_BLACK);
			}
		}
	}

	{ // Draw playhead
		float screenX = ToScreenPos(
			GraphPos(currentMode == MODE_PLAY ? state.playInfo.curGraphTime : state.playInfo.startGraphTime, 0),
			ctx).x;

		Vec topPoint = Vec(screenX, screenMin.y);
		float height = MIN(GetTopBarHeight(ctx) / 2.f, 8.f);

		// Playhead line
		Draw::PixelPerfectLine(topPoint, Vec(screenX, screenMax.y), COL_WHITE);
		Draw::PixelPerfectLine(Vec(screenX - 1, screenMin.y), Vec(screenX - 1, screenMax.y), COL_BLACK);
		Draw::PixelPerfectLine(Vec(screenX + 1, screenMin.y), Vec(screenX + 1, screenMax.y), COL_BLACK);

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