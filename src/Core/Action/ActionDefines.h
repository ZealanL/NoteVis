#pragma once
// This header file is responsible for defining all actions

#include "../Core.h"

#define MAKE_ACTION(name, func, bind, undoable) \
	INIT_FUNC([] { \
		string nameStr = #name; \
		g_Actions.push_back(Action(nameStr, func, bind, undoable)); \
	}, make_action_##name);

///////////////////////////////

MAKE_ACTION(Undo, [] {
	Core::UndoRestoreHistory();
	}, Action::Keybind(SDLK_z, KBFLAG_CTRL), false);

#pragma region Notes
MAKE_ACTION(DeleteSelectedNotes, [] {

	int deletedCount = g_NoteGraph.selectedNotes.size();
	if (g_NoteGraph.selectedNotes.size() < g_NoteGraph.GetNoteCount()) {
		while (!g_NoteGraph.selectedNotes.empty())
			g_NoteGraph.RemoveNote(*g_NoteGraph.selectedNotes.begin());

		g_NoteGraph.selectedNotes.clear();
	} else {
		// We are deleting all notes
		g_NoteGraph.ClearNotes();
	}

	}, Action::Keybind(SDLK_DELETE), true);

MAKE_ACTION(SelectAllNotes, [] {
	if (g_NoteGraph.selectedNotes.size() == g_NoteGraph.GetNoteCount()) {
		// If all notes are already selected, deselect everything
		g_NoteGraph.selectedNotes.clear();
		DLOG("Deselected all notes");
	} else {
		g_NoteGraph.selectedNotes.clear();
		for (Note* note : g_NoteGraph) {
			g_NoteGraph.selectedNotes.insert(note);
		}

		DLOG("Selected all notes");
	}

	}, Action::Keybind(SDLK_a, KBFLAG_CTRL), true);

MAKE_ACTION(InvertSelectedNotes, [] {

	// Determine min and max
	KeyInt lowestKey = KEY_AMOUNT;
	KeyInt highestKey = 0;
	for (Note* selected : g_NoteGraph.selectedNotes) {
		lowestKey = MIN(lowestKey, selected->key);
		highestKey = MAX(highestKey, selected->key);
	}

	if (highestKey <= lowestKey) {
		DLOG("Failed to invert selected notes (no keys selected?)");
	}

	for (Note* selected : g_NoteGraph.selectedNotes) {
		// Invert
		selected->key = highestKey - (selected->key - lowestKey);
		g_NoteGraph.CheckFixNoteOverlap(selected);
	}

	DLOG("Inverted {} notes, key range: {}", g_NoteGraph.selectedNotes.size(), (int)(highestKey - lowestKey));

	}, Action::Keybind(SDLK_i), true);


#pragma region Moving notes
#pragma region Single step
MAKE_ACTION(ShiftSelectedNotesUp, [] {

	g_NoteGraph.TryMoveSelectedNotes(0, 1);

	}, Action::Keybind(SDLK_UP), true);

MAKE_ACTION(ShiftSelectedNotesDown, [] {

	g_NoteGraph.TryMoveSelectedNotes(0, -1);

	}, Action::Keybind(SDLK_DOWN), true);

#pragma endregion
#pragma region Octave
MAKE_ACTION(ShiftSelectedNotesUpOctave, [] {

	g_NoteGraph.TryMoveSelectedNotes(0, KEYS_PER_OCTAVE);

	}, Action::Keybind(SDLK_UP, KBFLAG_CTRL), true);

MAKE_ACTION(ShiftSelectedNotesDownOctave, [] {

	g_NoteGraph.TryMoveSelectedNotes(0, -KEYS_PER_OCTAVE);

	}, Action::Keybind(SDLK_DOWN, KBFLAG_CTRL), true);
#pragma endregion
#pragma endregion
#pragma endregion

#undef MAKE_ACTION