#include "MenuBar.h"
#include "../../../Globals.h"
#include "../../Core.h"
#include "../UI.h"
#include "../../NVFileSystem/NVFileSystem.h"

void SubMenu(string name, std::function<void()> runFunc, bool enabled = true) {
	if (ImGui::BeginMenu(name.c_str(), enabled)) {
		runFunc();
		ImGui::EndMenu();
	}
}

// Option that triggers an Action from g_Actions
void Option(string actionName, bool enabled = true, string displayName = "") {
	auto action = g_Actions.actions[actionName];

	if (ImGui::MenuItem(displayName.empty() ? action->menuName.c_str() : displayName.c_str(), action->bind.ToString().c_str(), nullptr, enabled)) {
		action->Execute();
	}
}

// Option that is implemented inline (has no corresponding Action)
bool CustomOption(string name, bool enabled = true) {
	return ImGui::MenuItem(name.c_str(), nullptr, nullptr, enabled);
}

void MenuBar::Draw() {
	using namespace ImGui;
	BeginMainMenuBar();

	SubMenu("File",
		[] {

			if (CustomOption("New")) {
				if (NVFileSystem::TryCloseScore()) {
					NVFileSystem::SaveScoreAs();
				}
			}

			Option("Open");
			Option("Save");
			Option("SaveAs");

			Separator();

			if (CustomOption("Load MIDI")) {
				if (!g_NoteGraph.IsEmpty()) {
					// TODO: Implement
				} 
			}
		}
	);

	SubMenu("Edit",
		[] {
			Option("Undo", g_History.CanUndo());
			Option("Redo", g_History.CanRedo());
		}
	);

	SubMenu("Note Graph", 
		[] {
			Option("Play", true, g_NoteGraph.currentMode == NoteGraph::MODE_PLAY ? "Stop Playing" : "Play");

			Separator();

			if (CustomOption("Clear Notes", !g_NoteGraph.noteCache.notes.empty())) {
				NG_NOTIF("Removed all {} notes.", g_NoteGraph.noteCache.notes.size());
				g_NoteGraph.ClearNotes();
			}

			if (CustomOption("Clear Everything")) {
				g_NoteGraph.ClearEverything();
			}
		}
	);

	SubMenu("Notes", 
		[] {
			bool noteEditsEnabled = !g_NoteGraph.noteCache.selected.empty();

			bool allNotesSelected = !g_NoteGraph.noteCache.notes.empty() && g_NoteGraph.noteCache.notes.size() == g_NoteGraph.noteCache.selected.size();
			Option("SelectAllNotes", !g_NoteGraph.noteCache.notes.empty(), allNotesSelected ? "Deselect All" : "Select All");

			Separator();

			SubMenu("Move Key...", [=] {
				Option("ShiftSelectedNotesUp", noteEditsEnabled);
				Option("ShiftSelectedNotesDown", noteEditsEnabled);
				Option("ShiftSelectedNotesUpOctave", noteEditsEnabled);
				Option("ShiftSelectedNotesDownOctave", noteEditsEnabled);
				}, noteEditsEnabled);

			SubMenu("Move Time...", [=] {
				Option("ShiftSelectedNotesLeft", noteEditsEnabled);
				Option("ShiftSelectedNotesRight", noteEditsEnabled);
				Option("ShiftSelectedNotesLeftMeasure", noteEditsEnabled);
				Option("ShiftSelectedNotesRightMeasure", noteEditsEnabled);
				}, noteEditsEnabled);

			Option("DeleteSelectedNotes", noteEditsEnabled);
			Option("InvertSelectedNotes", noteEditsEnabled);
		}
	);

	EndMainMenuBar();
}