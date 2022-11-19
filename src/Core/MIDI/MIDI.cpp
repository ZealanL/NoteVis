#include "MIDI.h"
#include "MIDIPlayer/MIDIPlayer.h"

#include "../../midifile/MidiFile.h"
#include "MIDIFile/MIDIFile.h"

bool MIDI::ParseMidi(ByteDataStream::ReadIterator bytesIn, MIDIParseData& parseDataOut) {
	using namespace smf;

	if (bytesIn.stream->empty()) {
		DLOG("Failed to parse MIDI file (empty file)");
		return false; // Empty file
	}

	std::istringstream stream = std::istringstream((char*)&bytesIn.stream->front(), bytesIn.stream->size());
	MidiFile midiFile;
	if (!midiFile.read(stream)) {
		DLOG("Failed to parse MIDI file");
		return false;
	}
	
	// Keep track of how many times certain discrepencies occur in this MIDI file
	// These aren't fatal and don't prevent further parsing, but are a bad sign
	int outOfRangeNotes = 0, repeatedOnEvents = 0, 
		repeatedOffEvents = 0, invalidNotes = 0, unClosedNotes = 0;

	for (int i = 0; i < midiFile.getNumTracks(); i++) {

		MidiEventList& track = midiFile[i];
		
		// As we walk through the MIDI events, keep track of what note are being build (started playing)
		// first = is being built
		// second = note we are building
		constexpr int BUILDCACHE_SIZE = KEY_AMOUNT;
		pair<bool, Note> noteBuildCache[BUILDCACHE_SIZE]{};

		for (int j = 0; j < track.size(); j++) {
			MidiEvent& event = track[j];

			if (event.isNote()) {
				NoteTime time = event.tick * ((double)NOTETIME_PER_BEAT / (double)midiFile.getTicksPerQuarterNote());
	
				{
					KeyInt key = event.getKeyNumber() - KEYS_PER_OCTAVE; // MIDI is an octave above us

					int midiVel = event.getVelocity();
					VelInt vel = MIN(midiVel * 2, 255); // MIDI vel only goes to 127

					if (key >= BUILDCACHE_SIZE) {
						outOfRangeNotes++;
						continue;
					}

					// NOTE: Some MIDIs use NOTE_ON events with a velocity of zero instead of NOTE_OFF events
					bool on = event.isNoteOn();

					auto& cache = noteBuildCache[key];

					if (on) {
						if (cache.first)
							repeatedOnEvents++;

						cache.second.key = key;
						cache.second.time = time;
						cache.second.velocity = vel;
						cache.first = true;
					} else {
						if (cache.first) {
							cache.second.duration = time - cache.second.time;
							if (cache.second.IsValid()) {
								parseDataOut.notes.push_back(cache.second);
							} else {
								invalidNotes++;
							}
							cache.first = false;
						} else {
							repeatedOffEvents++;
						}
					}
				}
			}
		}

		for (auto& pair : noteBuildCache) {
			if (pair.first)
				unClosedNotes++;

			// TODO: Should we not ignore unclosed notes? Unsure.
		}
	}

	DLOG("Finished parsing MIDI, track count: {}, notes parsed: {}", 
		midiFile.getTrackCount(), parseDataOut.notes.size());

	if (outOfRangeNotes)
		DLOG("\t{} notes were out of range", outOfRangeNotes);
	if (repeatedOnEvents)
		DLOG("\t{} NOTE_ON events repeated", repeatedOnEvents);
	if (repeatedOffEvents)
		DLOG("\t{} NOTE_OFF events repeated", repeatedOffEvents);
	if (invalidNotes)
		DLOG("\t{} notes were invalid and disgarded", invalidNotes);
	if (unClosedNotes)
		DLOG("\t{} notes were unclosed at the end of the file", unClosedNotes);

	return true;
}

void MIDI::WriteMidi(MIDIParseData& parseDataIn, ByteDataStream& bytesOut) {
	bytesOut.clear();

	DLOG("Writing {} notes to MIDI file...", parseDataIn.notes.size());

	MIDIFile midiFile = MIDIFile();
	midiFile.ticksPerBeat = NOTETIME_PER_BEAT;
	midiFile.tracks.resize(1);

	MIDIFile::Track& track = midiFile.tracks[0];

	for (Note& note : parseDataIn.notes) {

		byte midiKey = note.key + KEYS_PER_OCTAVE;
		byte midiVel = CLAMP(note.velocity / 2, 1, 127);

		MIDIFile::TrackEvent noteOnEvent = {
			note.time,
			{
				(byte)MIDIMsgType::NOTE_ON, midiKey, midiVel
			}
		};

		MIDIFile::TrackEvent noteOffEvent = {
			note.time + note.duration,
			{
				(byte)MIDIMsgType::NOTE_ON, midiKey, 0
			}
		};

		track.push_back(noteOnEvent);
		track.push_back(noteOffEvent);
	}

	DLOG(" > Created {} MIDI events, serializing...", track.size());

	midiFile.Write(bytesOut);

	DLOG(" > Wrote {} bytes!", bytesOut.size());
}

vector<byte> MIDI::MakeVariableLengthBytes(uint32 amount) {
	// From https://github.com/jcelerier/libremidi/blob/6326ec688c9cc56cc418857c8e9b70ec9971377e/tests/unit/midifile_read.cpp

	uint8_t bytes[5] = { 0 };

	bytes[0] = (amount >> 28) & 0x7F; // most significant 5 bits
	bytes[1] = (amount >> 21) & 0x7F; // next largest 7 bits
	bytes[2] = (amount >> 14) & 0x7F;
	bytes[3] = (amount >> 7) & 0x7F;
	bytes[4] = (amount) & 0x7F; // least significant 7 bits

	int start = 0;
	while (start < 5 && bytes[start] == 0)
		start++;

	vector<byte> outBytes;

	for (int i = start; i < 4; i++) {
		bytes[i] = bytes[i] | 0x80;
		outBytes.push_back(bytes[i]);
	}
	outBytes.push_back(bytes[4]);
	return outBytes;
}