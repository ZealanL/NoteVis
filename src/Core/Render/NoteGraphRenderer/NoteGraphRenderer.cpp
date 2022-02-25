#include "NoteGraphRenderer.h"
#include "../Draw/Draw.h"
#include "../../Note/NoteColors.h"

void NoteGraphRenderer::DrawNoteGraph(NoteGraph& graph, Area screenArea) {
	float noteSize = floorf(graph.vScale / KEY_AMOUNT);

	Draw::Rect(screenArea.min, screenArea.max, COL_BLACK);

	float minDrawX = MAX(screenArea.min.x, graph.ToScreenPos(0, 0, screenArea.Size()).x);

	// Horizontal slot lines
	for (int i = 0; i < KEY_AMOUNT; i++) {
		auto screenY = screenArea.min.y + graph.ToScreenPos(0, i, screenArea.Size()).y;

		Draw::PixelPerfectLine(Vec(minDrawX, screenY), Vec(screenArea.max.x, screenY), Color(25, 25, 25));
	}

	// Vertical lines lines
	for (int i = 0; i < 100; i++) {
		float beat = i / 4.f;
		NoteTime t = beat * NOTETIME_PER_BEAT;
		bool isBeatLine = (beat == roundf(beat));
		bool isMeasureLine = isBeatLine && ((int)beat % graph.timeSig.num) == 0;

		// More alpha = more important line
		int alpha = isMeasureLine ? 80 : (isBeatLine ? 45 : 15);

		auto screenX = screenArea.min.x + graph.ToScreenPos(t, 0, screenArea.Size()).x;

		Draw::PixelPerfectLine(Vec(screenX, screenArea.min.y), Vec(screenX, screenArea.max.y),
			Color(255, 255, 255, alpha));
	}


	{ // Draw notes

		// We need to draw notes in order of time, this will contain note pointers but sorted
		vector<Note*> notesToDraw;

		for (Note* note : graph) {
			// TODO: Sub-optimal check requires function calls
			Vec screenPos = screenArea.min + graph.ToScreenPos(note->time, note->key, screenArea.Size());
			float tailEndX = screenArea.min.x + graph.ToScreenPos(note->time + note->duration, 0, screenArea.Size()).x;

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

		bool anySelected = !graph.selectedNotes.empty();

		for (Note* note : notesToDraw) {
			bool selected = graph.IsNoteSelected(note);

			Vec screenPos = screenArea.min + graph.ToScreenPos(note->time, note->key, screenArea.Size());
			float tailEndX = screenArea.min.x + graph.ToScreenPos(note->time + note->duration, 0, screenArea.Size()).x;

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
}