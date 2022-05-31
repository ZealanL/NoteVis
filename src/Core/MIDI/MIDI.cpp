#include "MIDI.h"
#include "MIDIPlayer/MIDIPlayer.h"
#include "../../libremidi/writer.hpp"
#include "../../libremidi/reader.hpp"
#include "../../libremidi/libremidi.hpp"
using namespace libremidi;

bool MIDI::ParseMidi(ByteDataStream::ReadIterator bytesIn, MIDIParseData& parseDataOut) {
	reader midiReader = reader(true);

	reader::parse_result result = midiReader.parse(bytesIn.stream->GetBasePointer() + bytesIn.curIndex, bytesIn.BytesLeft());
	switch (result) {
	case reader::incomplete:
		DLOG("Failed to parse MIDI file (incomplete)");
		return false;
	case reader::invalid:
		DLOG("Failed to parse MIDI file (invalid)");
		return false;
	}

	// Keep track of how many times certain discrepencies occur in this MIDI file
	// These aren't fatal and don't prevent further parsing, but are a bad sign
	int outOfRangeNotes = 0, repeatedOnEvents = 0, 
		repeatedOffEvents = 0, invalidNotes = 0, unClosedNotes = 0;

	// Reference: https://github.com/jcelerier/libremidi/blob/master/tests/midifile_dump.cpp#L57
	for (auto& track : midiReader.tracks) {

		// As we walk through the MIDI events, keep track of what note are being build (started playing)
		// first = is being built
		// second = note we are building
		constexpr int BUILDCACHE_SIZE = KEY_AMOUNT;
		pair<bool, Note> noteBuildCache[BUILDCACHE_SIZE]{};

		for (auto& event : track) {
			if (!event.m.is_meta_event()) {
				NoteTime time = event.tick * ((double)NOTETIME_PER_BEAT / (double)midiReader.ticksPerBeat);
				message_type type = event.m.get_message_type();

				if (type == message_type::NOTE_ON || type == message_type::NOTE_OFF) {
					KeyInt key = event.m.bytes[1] - KEYS_PER_OCTAVE; // MIDI is an octave above us

					int midiVel = event.m.bytes[2];
					VelInt vel = MIN(midiVel * 2, 255); // MIDI vel only goes to 127

					if (key >= BUILDCACHE_SIZE) {
						outOfRangeNotes++;
						continue;
					}

					// NOTE: Some MIDIs use NOTE_ON events with a velocity of zero instead of NOTE_OFF events
					bool on = type == message_type::NOTE_ON && vel > 0;

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

	DLOG("Finished parsing MIDI, track count: {}, tick length: {}, notes parsed: {}", 
		midiReader.tracks.size(), (int)midiReader.get_end_time(), parseDataOut.notes.size());

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
	writer midiWriter;
	midiWriter.ticksPerQuarterNote = NOTETIME_PER_BEAT;

	midiWriter.tracks.resize(1);
	midi_track& track = midiWriter.tracks[0];

	vector<Note> sortedNotes = parseDataIn.notes;
	std::sort(sortedNotes.begin(), sortedNotes.end(), [](Note& a, Note& b) {
		return a.time < b.time;
	});

	NoteTime lastNoteEndTime = 0;
	for (Note& note : sortedNotes) {
		NoteTime timePad = note.time - (lastNoteEndTime);
		lastNoteEndTime = note.time + note.duration;

		track.push_back(track_event{
			timePad, 0, message::note_on(1, note.key, note.velocity)
			});

		track.push_back(track_event{
			note.duration, 0, message::note_off(1, note.key, note.velocity)
			});

		lastNoteEndTime = note.time + note.duration;
	}

	// libremidi requires an ostream, so we must make one...
	// TODO: This is kind of messy, can it be made cleaner?
	std::stringbuf tempBuf;
	std::iostream tempStream = std::iostream(&tempBuf);

	midiWriter.write(tempStream);

	bytesOut.insert(
		bytesOut.end(),
		std::istream_iterator<char>(tempStream), 
		std::istream_iterator<char>()
	);
}