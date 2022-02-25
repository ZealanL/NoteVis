// This header file is responsible for defining all actions

#pragma once
#include "../Core.h"

#define MAKE_ACTION(name, func, bind) \
	INIT_FUNC([] { \
		string nameStr = #name; \
		g_Actions.push_back(Action(nameStr, func, bind)); \
	}, make_action_##name);


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

	DLOG("Deleted " << deletedCount << " notes");

}, Action::Keybind( SDLK_DELETE ));

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

	}, Action::Keybind(SDLK_a, KBFLAG_CTRL));

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
	}

	DLOG("Inverted " << g_NoteGraph.selectedNotes.size() << " notes, key range: " << (int)(highestKey - lowestKey));

	}, Action::Keybind(SDLK_i));


#pragma region Moving notes
#pragma region Single step
MAKE_ACTION(ShiftSelectedNotesUp, [] {

	for (Note* selected : g_NoteGraph.selectedNotes)
		if (selected->key == KEY_AMOUNT - 1)
			return; // Can't move any futher

	for (Note* selected : g_NoteGraph.selectedNotes)
		selected->key++;

	DLOG("Moved " << g_NoteGraph.selectedNotes.size() << " notes up");

	}, Action::Keybind( SDLK_UP ));

MAKE_ACTION(ShiftSelectedNotesDown, [] {

	for (Note* selected : g_NoteGraph.selectedNotes)
		if (selected->key == 0)
			return; // Can't move any futher

	for (Note* selected : g_NoteGraph.selectedNotes)
		selected->key--;

	DLOG("Moved " << g_NoteGraph.selectedNotes.size() << " notes down");

	}, Action::Keybind( SDLK_DOWN ));
#pragma endregion
#pragma region Octave
MAKE_ACTION(ShiftSelectedNotesUpOctave, [] {

	for (Note* selected : g_NoteGraph.selectedNotes)
		if (selected->key >= KEY_AMOUNT - KEYS_PER_OCTAVE)
			return; // Can't move any futher

	for (Note* selected : g_NoteGraph.selectedNotes)
		selected->key += KEYS_PER_OCTAVE;

	DLOG("Moved " << g_NoteGraph.selectedNotes.size() << " notes an octave up");

	}, Action::Keybind( SDLK_UP, KBFLAG_CTRL ));

MAKE_ACTION(ShiftSelectedNotesDownOctave, [] {

	for (Note* selected : g_NoteGraph.selectedNotes)
		if (selected->key < KEYS_PER_OCTAVE)
			return; // Can't move any futher

	for (Note* selected : g_NoteGraph.selectedNotes)
		selected->key -= KEYS_PER_OCTAVE;

	DLOG("Moved " << g_NoteGraph.selectedNotes.size() << " notes an octave down");

	}, Action::Keybind( SDLK_DOWN, KBFLAG_CTRL ));
#pragma endregion
#pragma endregion
#pragma endregion
#undef MAKE_ACTION