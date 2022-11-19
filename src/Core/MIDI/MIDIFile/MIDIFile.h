#include "../../../Types/ByteDataSteam.h"

#include "../MIDI.h"

struct MIDIFile {
	// NOTE: Negative values use SMPTE units
	int16 ticksPerBeat = 120;

	struct TrackEvent {
		uint32 tick; // Absolute tick time
		MIDIMessage msg;
	};

	typedef vector<TrackEvent> Track;
	vector<Track> tracks;

	bool Write(ByteDataStream& bytesOut);
};