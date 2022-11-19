#include "MIDIPlayer.h"

struct MIDIMessage {
	byte data[4] = { 0, 0, 0, 0 };

	MIDIMessage(std::initializer_list<byte> data) {
		ASSERT(data.size() <= 4);
		memcpy(this->data, data.begin(), data.size());
	}

	uint32 AsInt() {
		return *(uint32*)(&data);
	}
};

MIDIPlayer::MIDIPlayer() {
#ifdef PLAT_WINDOWS
	winMidiDevice = NULL;

	MMRESULT result = midiOutOpen(&winMidiDevice, 0, NULL, NULL, CALLBACK_NULL);
	if (result != MMSYSERR_NOERROR) {
		FW::FatalError("MIDI Player failed to open MIDI output (error code: {}).", result);
	}
#endif
}

MIDIPlayer::~MIDIPlayer() {
	StopAll();
#ifdef PLAT_WINDOWS
	if (winMidiDevice)
		midiOutClose(winMidiDevice);
#endif
}

void MIDIPlayer::NoteOn(KeyInt key, VelInt vel) {
	auto msg = MIDIMessage({ 0x90, (byte)(key + KEYS_PER_OCTAVE), (byte)CLAMP(1, 127, vel / 2), NULL });
#ifdef PLAT_WINDOWS
	midiOutShortMsg(winMidiDevice, msg.AsInt());
#endif
}

void MIDIPlayer::NoteOff(KeyInt key) {
	auto msg = MIDIMessage({ 0x90, (byte)(key + KEYS_PER_OCTAVE), NULL, NULL });
#ifdef PLAT_WINDOWS
	midiOutShortMsg(winMidiDevice, msg.AsInt());
#endif
}

void MIDIPlayer::StopAll() {
#ifdef PLAT_WINDOWS
	ASSERT(winMidiDevice);
	midiOutReset(winMidiDevice);
#endif
}