#pragma once
#include "../NoteTypes.h"
#include "../../../Types/ByteDataSteam.h"
#include "../../UI/LogNotif/LogNotif.h"
#include "../NoteStorageCache/NoteStorageCache.h"

struct GraphPos {
	NoteTime x = 0;
	float y = 0;

	GraphPos() = default;
	GraphPos(NoteTime x, float y) : x(x), y(y) {}

	friend bool operator==(const GraphPos& a, const GraphPos& b) {
		return a.x == b.x && a.y == b.y;
	}

	friend bool operator!=(const GraphPos& a, const GraphPos& b) {
		return !(a == b);
	}
};

// NOTE: Rendering of the NoteGraph is Rendering/NoteGraphRender
class NoteGraph {
public:
	NoteStorageCache noteCache;
	LogNotifList logNotifs;

	// Current mode
	enum {
		MODE_IDLE,

		MODE_RECTSELECT, // Use a rectangle shape to select notes

		MODE_DRAGNOTES, // Drag the selected note(s) around on the graph
		MODE_DRAGNOTELENGTHS, // Drag the length of the selected note(s)

		MODE_ADJUSTNOTEVEL, // Adjust the velocity of the selected note(s)

		MODE_PLAY
	};
	int currentMode = MODE_IDLE;

	struct NoteGraphInputState {
		bool mouseDown = false;

		struct {
			Note* startDragSelectedNote;
			GraphPos startDragPos;
			Vec startDragMousePos;
			bool selectedNoteLastMouseDown = false;
		} dragInfo;

		struct {
			double playStartTime;
			NoteTime startGraphTime;
			NoteTime curGraphTime;
		} playInfo;
		
	};
	NoteGraphInputState state;

	NoteTime GetFurthestNoteEndTime() {
		return noteCache.GetFurthestNoteEndTime();
	}

	NoteTime GetGraphEndTime() {
		return MAX(noteCache.GetFurthestNoteEndTime(), NOTETIME_PER_BEAT * timeSig.beatCount);
	}

	Note* hoveredNote = NULL;

	bool IsNoteSelected(Note* note) {
		return noteCache.selected.count(note);
	}

	bool IsEmpty() {
		return noteCache.notes.empty();
	}

	struct TimeSignature {
		int beatCount = 4;
		// TODO: Denominator so we can support 7/8 and such
	};
	TimeSignature timeSig;

	// Horizontal scroll of the note graph
	NoteTime hScroll = 0;

	// Horizontal zoom, Pixels per NOTETIME_PER_BEAT NoteTime
	float hZoom = 100.f;

	// Vertical scale/zoom, how high the full notegraph is relative to screen height 
	float vScale = 1.f;

	// Time step to snap to
	int snappingTime = NOTETIME_PER_BEAT / 4;

	// Returns true if was handled
	bool TryHandleSpecialKeyEvent(SDL_Keycode key, byte kbFlags);

	struct RenderContext {
		Area fullNoteGraphScreenArea;
	};

	float GetTopBarHeight(RenderContext* ctx);
	float GetBottomBarHeight(RenderContext* ctx);
	Area GetNoteAreaScreen(RenderContext* ctx);
    double GetNoteTimePerPx();
    Vec ToScreenPos(GraphPos graphPos, RenderContext* ctx);
	GraphPos ToGraphPos(Vec screenPos, RenderContext* ctx);

	// Fix any potential overlap with this note, will only modify/remove other notes
    void CheckFixNoteOverlap(Note* note);

	// ignoreOverlap: Don't call CheckFixNoteOverlap after moving
	void MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap = false);

	void TogglePlay();

	int GetNoteCount();
	void ClearEverything(bool notify = true);
	auto GetNotes() { return noteCache.notes; }
	Note* AddNote(Note note, bool ignoreOverlap = false); // Returns pointer to added note
	bool RemoveNote(Note* note); // Returns true if note was found and removed
	void ClearNotes();

	void AddMIDIData(struct MIDIParseData& midiDataOut);
	void MakeMIDIData(struct MIDIParseData& midiDataOut);

	void Render(RenderContext* ctx);

private:
	void RenderNotes(RenderContext* ctx);
public:

	void UpdateWithInput(SDL_Event& e, RenderContext* ctx);

	// Returns true if they could be moved, false if not
	// ignoreOverlap: Don't call CheckFixNoteOverlap after moving
	bool TryMoveSelectedNotes(int amountX, int amountY, bool ignoreOverlap = false);

	void Serialize(ByteDataStream& bytesOut);
	void Deserialize(ByteDataStream::ReadIterator bytesIn);

	// Base size for each note's head, in screen pixels
	int GetNoteBaseHeadSizeScreen(RenderContext* ctx);

    void UpdatePlayOnRender();

	void DrawScrollBar(RenderContext* ctx);

	// For C++ iterator
	auto begin() { return noteCache.begin(); }
	auto end() { return noteCache.end(); }
};