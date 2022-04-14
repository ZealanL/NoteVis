#pragma once
// MIDI is for interacting with MIDI IO

#include "../../Framework.h"
#include "../../Types/ByteDataSteam.h"

namespace MIDI {
	libremidi::reader::parse_result ParseMidi(ByteDataSteam::ReadIterator bytesIn);
}