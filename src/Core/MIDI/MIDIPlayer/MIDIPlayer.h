#pragma once
#include "../../Note/NoteTypes.h"

#ifdef PLAT_WINDOWS
#include <mmsystem.h> // Windows MultiMedia System
#endif

struct MIDIPlayer {
#ifdef PLAT_WINDOWS
	HMIDIOUT winMidiDevice;
#endif

	MIDIPlayer();
	~MIDIPlayer();

	void NoteOn(KeyInt key, VelInt vel);
	void NoteOff(KeyInt key);
	void StopAll();
};