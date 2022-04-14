#pragma once
// MIDI is for interacting with MIDI IO

#include "../../Framework.h"
#include "../../Types/ByteDataSteam.h"
#include "../Note/NoteTypes.h"

struct MIDIParseData {
	vector<Note> notes;

	// TODO: Tempo information, etc.
};

namespace MIDI {
	bool ParseMidi(ByteDataStream::ReadIterator bytesIn, MIDIParseData& parseDataOut);
}