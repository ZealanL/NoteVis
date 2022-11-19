#include "MIDIFile.h"
#include "../MIDI.h"

// References:
//	http://www.somascape.org/midi/tech/mfile.html
//	http://www.ccarh.org/courses/253/handout/smf/

#define MIDI_HEADER_MAGIC 'MThd'
#define MIDI_TRACK_MAGIC 'MTrk'

enum class MIDIFileFormat : uint16 {
	SINGLE_TRACK,
	MULTI_TRACK,
	MULTI_SONG,
};

bool MIDIFile::Write(ByteDataStream& bytesOut) {
	// Write MIDI file header
	{
		// Magic
		bytesOut.WriteInv<uint32>(MIDI_HEADER_MAGIC);

		// Remaining header size
		bytesOut.WriteInv<uint32>(6);

		// Format
		if (tracks.size() > 1) {
			bytesOut.WriteInv(MIDIFileFormat::SINGLE_TRACK);
		} else {
			bytesOut.WriteInv(MIDIFileFormat::MULTI_TRACK);
		}
		
		// Chunk count
		bytesOut.WriteInv<uint16>(tracks.size());

		// Ticks per beat
		bytesOut.WriteInv(this->ticksPerBeat);
	}

	for (Track track : tracks) {

		// Sort track
		std::stable_sort(track.begin(), track.end(), 
			[](const TrackEvent& a, const TrackEvent& b) {
				return a.tick < b.tick;
			}
		);

		// Write track data
		ByteDataStream trackData;
		uint32 lastTick = 0;
		for (TrackEvent& event : track) {
			if (event.msg.empty())
				continue;

			// Calculate and write variable length distance
			uint32 tickDelta = event.tick - lastTick;
			vector<byte> variableLengthBytes = MIDI::MakeVariableLengthBytes(tickDelta);
			trackData.insert(trackData.end(), variableLengthBytes.begin(), variableLengthBytes.end());

			// Write message bytes
			trackData.insert(trackData.end(), event.msg.begin(), event.msg.end());

			lastTick = event.tick;
		}

		// Add track end message
		{
			trackData.push_back(0);
			trackData.push_back(0xFF);
			trackData.push_back(0x2F);
			trackData.push_back(0);
		}

		// Write track header
		{
			// Magic
			bytesOut.WriteInv(MIDI_TRACK_MAGIC);
			
			// Length
			bytesOut.WriteInv<uint32>(trackData.size());
		}

		// Add track bytes
		bytesOut.insert(bytesOut.end(), trackData.begin(), trackData.end());
	}

	return true;
}