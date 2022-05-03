// This cpp file is responsible for defining all actions

#include "ActionDefineSystem.h"

#include "../../Core.h"
#include "../../../Globals.h"

#define MAKE_ACTION(name, func, bind, undoable) this->actions.push_back(new Action(#name, func, bind, undoable))

// NOTE: All hold actions are class properties as well
#define MAKE_HOLDACTION(name, bind) this->ha_##name = new HoldAction(#name, bind); this->holdActions.push_back(this->ha_##name)

void ActionDefineSystem::Init() {
	// Should not initialize more than one
	ASSERT(actions.empty() && holdActions.empty());

	DLOG("Initializing ActionDefineSystem...");

	///////////////////////////////

#pragma region Action Definitions
	MAKE_ACTION(Undo, [] {
		g_History.Undo();
		}, Keybind(SDLK_z, KBFLAG_CTRL), false);

	MAKE_ACTION(Redo, [] {
		g_History.Redo();
		}, Keybind(SDLK_y, KBFLAG_CTRL), false);

#pragma region Notes
	MAKE_ACTION(DeleteSelectedNotes, [] {

		int deletedCount = g_NoteGraph.noteCache.selected.size();
		if (g_NoteGraph.noteCache.selected.size() < g_NoteGraph.GetNoteCount()) {
			while (!g_NoteGraph.noteCache.selected.empty())
				g_NoteGraph.RemoveNote(*g_NoteGraph.noteCache.selected.begin());

			g_NoteGraph.noteCache.DeselectAll();
		} else {
			// We are deleting all notes
			g_NoteGraph.ClearNotes();
		}

		}, Keybind(SDLK_DELETE), true);

	MAKE_ACTION(SelectAllNotes, [] {
		if (g_NoteGraph.noteCache.selected.size() == g_NoteGraph.GetNoteCount()) {
			// If all notes are already selected, deselect everything
			g_NoteGraph.noteCache.DeselectAll();
			DLOG("Deselected all notes");
		} else {
			for (Note* note : g_NoteGraph)
				g_NoteGraph.noteCache.SetSelected(note, true);

			DLOG("Selected all notes");
		}

		}, Keybind(SDLK_a, KBFLAG_CTRL), true);

	MAKE_ACTION(InvertSelectedNotes, [] {

		// Determine min and max
		KeyInt lowestKey = KEY_AMOUNT;
		KeyInt highestKey = 0;
		for (Note* selected : g_NoteGraph.noteCache.selected) {
			lowestKey = MIN(lowestKey, selected->key);
			highestKey = MAX(highestKey, selected->key);
		}

		if (highestKey <= lowestKey) {
			DLOG("Failed to invert selected notes (no keys selected?)");
		}

		// Invert
		for (Note* selected : g_NoteGraph.noteCache.selected)
			g_NoteGraph.MoveNote(selected, selected->time, highestKey - (selected->key - lowestKey), true);

		// Check overlap
		for (Note* selected : g_NoteGraph.noteCache.selected)
			g_NoteGraph.CheckFixNoteOverlap(selected);

		NG_NOTIF("Inverted {} notes (key range: {})", g_NoteGraph.noteCache.selected.size(), (int)(highestKey - lowestKey));

		}, Keybind(SDLK_i), true);

#pragma region Moving notes

#pragma region Single step
	MAKE_ACTION(ShiftSelectedNotesLeft, [] {
		g_NoteGraph.TryMoveSelectedNotes(-g_NoteGraph.snappingTime, 0);
		}, Keybind(SDLK_LEFT), true);

	MAKE_ACTION(ShiftSelectedNotesRight, [] {
		g_NoteGraph.TryMoveSelectedNotes(g_NoteGraph.snappingTime, 0);
		}, Keybind(SDLK_RIGHT), true);

	MAKE_ACTION(ShiftSelectedNotesUp, [] {
		g_NoteGraph.TryMoveSelectedNotes(0, 1);
		}, Keybind(SDLK_UP), true);

	MAKE_ACTION(ShiftSelectedNotesDown, [] {
		g_NoteGraph.TryMoveSelectedNotes(0, -1);
		}, Keybind(SDLK_DOWN), true);

#pragma endregion

#pragma region Octave
	MAKE_ACTION(ShiftSelectedNotesUpOctave, [] {
		g_NoteGraph.TryMoveSelectedNotes(0, KEYS_PER_OCTAVE);
		}, Keybind(SDLK_UP, KBFLAG_CTRL), true);

	MAKE_ACTION(ShiftSelectedNotesDownOctave, [] {
		g_NoteGraph.TryMoveSelectedNotes(0, -KEYS_PER_OCTAVE);
		}, Keybind(SDLK_DOWN, KBFLAG_CTRL), true);
#pragma endregion

#pragma region Measure
	MAKE_ACTION(ShiftSelectedNotesLeftMeasure, [] {
		g_NoteGraph.TryMoveSelectedNotes(-g_NoteGraph.timeSig.beatCount * NOTETIME_PER_BEAT, 0);
		}, Keybind(SDLK_LEFT, KBFLAG_CTRL), true);

	MAKE_ACTION(ShiftSelectedNotesRightMeasure, [] {
		g_NoteGraph.TryMoveSelectedNotes(g_NoteGraph.timeSig.beatCount * NOTETIME_PER_BEAT, 0);
		}, Keybind(SDLK_RIGHT, KBFLAG_CTRL), true);
#pragma endregion

#pragma endregion
#pragma endregion
#pragma endregion

#pragma region HoldAction Definitions
	MAKE_HOLDACTION(ChangeNoteVels, Keybind(SDLK_v));
#pragma endregion

	///////////////////////////////

	DLOG("\tAdded {} actions and {} hold actions", actions.size(), holdActions.size());
}