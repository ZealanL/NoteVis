// This cpp file is responsible for defining all actions

#include "ActionDefineSystem.h"

#include "../../Core.h"
#include "../../../Globals.h"

#include "../../NVFileSystem/NVFileSystem.h"

#define MAKE_ACTION(name, menuName, func, bind, undoable) this->actions[#name] = new Action(#name, menuName, func, bind, undoable)

// NOTE: All hold actions are class properties as well
#define MAKE_HOLDACTION(name, bind) this->holdActions[#name] = new HoldAction(#name, bind);

void ActionDefineSystem::Init() {

	// Should not initialize more than one
	ASSERT(actions.empty() && holdActions.empty());

	DLOG("Initializing ActionDefineSystem...");

	///////////////////////////////

#pragma region Action Definitions

	MAKE_ACTION(Open, "Open", [] {
		NVFileSystem::OpenScore();
		}, Keybind(SDLK_o, KBFLAG_CTRL), false);

	MAKE_ACTION(Save, "Save", [] {
		NVFileSystem::SaveScore();
		}, Keybind(SDLK_s, KBFLAG_CTRL), false);

	MAKE_ACTION(SaveAs, "Save As...", [] {
		NVFileSystem::SaveScoreAs();
		}, Keybind(SDLK_s, KBFLAG_CTRL | KBFLAG_SHIFT), false);

	MAKE_ACTION(Undo, "Undo", [] {
		g_History.Undo();
		}, Keybind(SDLK_z, KBFLAG_CTRL), false);

	MAKE_ACTION(Redo, "Redo", [] {
		g_History.Redo();
		}, Keybind(SDLK_y, KBFLAG_CTRL), false);

	MAKE_ACTION(Play, "Play", [] {
		g_NoteGraph.TogglePlay();
		}, Keybind(SDLK_SPACE), false);

#pragma region Notes
	MAKE_ACTION(SelectAllNotes, "Select All", [] {
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

	MAKE_ACTION(DeleteSelectedNotes, "Delete", [] {

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

	MAKE_ACTION(InvertSelectedNotes, "Invert", [] {

		// Determine min and max
		KeyInt lowestKey = KEY_AMOUNT;
		KeyInt highestKey = 0;
		for (Note* selected : g_NoteGraph.noteCache.selected) {
			lowestKey = MIN(lowestKey, selected->key);
			highestKey = MAX(highestKey, selected->key);
		}

		if (highestKey <= lowestKey) {
			DLOG("Failed to invert selected notes (no keys selected?)");
			return;
		}

		// Invert
		for (Note* selected : g_NoteGraph.noteCache.selected)
			g_NoteGraph.MoveNote(selected, selected->time, highestKey - (selected->key - lowestKey), true);
		
		// Check overlap
			for (Note* selected : g_NoteGraph.noteCache.selected)
				g_NoteGraph.CheckFixNoteOverlap(selected);

		}, Keybind(SDLK_i), true);

#pragma region Moving notes

#pragma region Single step
	MAKE_ACTION(ShiftSelectedNotesLeft, "Move left", [] {
		g_NoteGraph.TryMoveSelectedNotes(-g_NoteGraph.snappingTime, 0);
		}, Keybind(SDLK_LEFT), true);

	MAKE_ACTION(ShiftSelectedNotesRight, "Move right", [] {
		g_NoteGraph.TryMoveSelectedNotes(g_NoteGraph.snappingTime, 0);
		}, Keybind(SDLK_RIGHT), true);

#pragma region Measure Move
	MAKE_ACTION(ShiftSelectedNotesLeftMeasure, "Move a measure back", [] {
		g_NoteGraph.TryMoveSelectedNotes(-g_NoteGraph.timeSig.beatCount * NOTETIME_PER_BEAT, 0);
		}, Keybind(SDLK_LEFT, KBFLAG_CTRL), true);

	MAKE_ACTION(ShiftSelectedNotesRightMeasure, "Move a measure forward", [] {
		g_NoteGraph.TryMoveSelectedNotes(g_NoteGraph.timeSig.beatCount * NOTETIME_PER_BEAT, 0);
		}, Keybind(SDLK_RIGHT, KBFLAG_CTRL), true);
#pragma endregion

	MAKE_ACTION(ShiftSelectedNotesUp, "Step up", [] {
		g_NoteGraph.TryMoveSelectedNotes(0, 1);
		}, Keybind(SDLK_UP), true);

	MAKE_ACTION(ShiftSelectedNotesDown, "Step down", [] {
		g_NoteGraph.TryMoveSelectedNotes(0, -1);
		}, Keybind(SDLK_DOWN), true);

#pragma endregion

#pragma region Octave
	MAKE_ACTION(ShiftSelectedNotesUpOctave, "Move an octave up", [] {
		g_NoteGraph.TryMoveSelectedNotes(0, KEYS_PER_OCTAVE);
		}, Keybind(SDLK_UP, KBFLAG_CTRL), true);

	MAKE_ACTION(ShiftSelectedNotesDownOctave, "Move an octave down", [] {
		g_NoteGraph.TryMoveSelectedNotes(0, -KEYS_PER_OCTAVE);
		}, Keybind(SDLK_DOWN, KBFLAG_CTRL), true);
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