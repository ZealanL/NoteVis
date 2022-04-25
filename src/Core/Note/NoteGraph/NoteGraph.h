#pragma once
#include "../NoteTypes.h"
#include "../../../Types/ByteDataSteam.h"



struct GraphPos {
	NoteTime x;
	float y;

	friend bool operator==(const GraphPos& a, const GraphPos& b) {
		return a.x == b.x && a.y == b.y;
	}
};

// NOTE: Rendering of the NoteGraph is Rendering/NoteGraphRender
class NoteGraph {
private:
	// Private to prevent manual modification, this would cause serious desyncronization issues
	set<Note*> _notes;
	NoteTime _furthestNoteEndTime;

	// Keep track of what notes are in what key slot for fast comparisons
	set<Note*> _noteSlots[KEY_AMOUNT];
public:
	// Current mode
	enum {
		MODE_IDLE,
		MODE_RECTSELECT,
		MODE_DRAGNOTES,
		MODE_DRAGNOTELENGTHS,
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
			NoteTime startTime;
			NoteTime curTime;
		} playInfo;
		
	};
	NoteGraphInputState state;

	NoteTime GetFurthestNoteEndTime() {
		return _furthestNoteEndTime;
	}

	NoteTime GetGraphEndTime() {
		return MAX(_furthestNoteEndTime, NOTETIME_PER_BEAT * timeSig.beatCount);
	}

	Note* hoveredNote = NULL;

	set<Note*> selectedNotes; // Only contains pointers already in _notes 
	bool IsNoteSelected(Note* note) {
		return selectedNotes.find(note) != selectedNotes.end();
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
	bool TryHandleSpecialKeyEvent(SDL_Keycode key, BYTE kbFlags);

	struct RenderContext {
		Area fullNoteGraphScreenArea;
	};
	float GetTopBarHeight(RenderContext* ctx);
	Area GetNoteAreaScreen(RenderContext* ctx);
    Vec ToScreenPos(GraphPos graphPos, RenderContext* ctx);
	GraphPos ToGraphPos(Vec screenPos, RenderContext* ctx);

	// Fix any potential overlap with this note, will only modify/remove other notes
    void CheckFixNoteOverlap(Note* note);

	// ignoreOverlap: Don't call CheckFixNoteOverlap after moving
	void MoveNote(Note* note, NoteTime newX, KeyInt newY, bool ignoreOverlap = false);

	int GetNoteCount();
	void ClearEverything();
	const set<Note*>& GetNotes() { return _notes; }
	Note* AddNote(Note note, bool ignoreOverlap = false); // Returns pointer to added note
	bool RemoveNote(Note* note); // Returns true if note was found and removed
	void ClearNotes();


	void Render(RenderContext* ctx);
private:
	void RenderNotes(RenderContext* ctx);

public:
	void UpdateWithInput(SDL_Event& e, RenderContext* ctx);

	// Returns true if they could be moved, false if not
	// ignoreOverlap: Don't call CheckFixNoteOverlap after moving
	bool TryMoveSelectedNotes(int amountX, int amountY, bool ignoreOverlap = false);

	void Serialize(ByteDataStream& bytesOut);
	void Deserialize(ByteDataStream::ReadIterator& bytesIn);

	// Base size for each note's head, in screen pixels
	int GetNoteBaseHeadSizeScreen(RenderContext* ctx);

	~NoteGraph() {
		for (Note* note : _notes)
			delete note;
	}

	// For C++ iterator
	auto begin() { return _notes.begin(); }
	auto end() { return _notes.end(); }
};