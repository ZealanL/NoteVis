#pragma once
// MIDI is for interacting with MIDI IO

#include "../../Framework.h"
#include "../../Types/ByteDataSteam.h"
#include "../Note/NoteTypes.h"

enum class MIDIMsgType : byte {
	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
};

typedef vector<byte> MIDIMessage;

struct MIDIParseData {
	vector<Note> notes;

	// TODO: Tempo information, etc.
};

namespace MIDI {
	bool ParseMidi(ByteDataStream::ReadIterator bytesIn, MIDIParseData& parseDataOut);
    void WriteMidi(MIDIParseData& parseDataIn, ByteDataStream& bytesOut);

	// Make a variable-length encoded byte sequence for MIDI events
	vector<byte> MakeVariableLengthBytes(uint32 amount);
}