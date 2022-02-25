// Note type and related types
#pragma once
#include "../../Framework.h"
#include "../Types/Color.h"

#define KEYS_PER_OCTAVE 12
#define KEY_AMOUNT (88 + 9) // Most pianos have 88, I have extended it down to a very low C such that an index 0 key is on C

// Represnted in milliseconds at 60 BPM
typedef uint32_t NoteTime;

// NoteTime each beat (60 BPM ms = 1000 NOTETIME_PER_BEAT)
#define NOTETIME_PER_BEAT 1000

// Integer representing key index
// Always should be < TOTAL_KEYS
typedef uint8_t KeyInt;

// Note velocity, from 0 to 255
typedef uint8_t NoteVelInt;

#pragma region KeyNoteMask
typedef const bool KeyNoteMask[KEYS_PER_OCTAVE];

constexpr KeyNoteMask KEYMASK_MAJOR = {
	1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1
};

constexpr KeyNoteMask KEYMASK_MINOR = {
	1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0
};

constexpr KeyNoteMask KEYMASK_HARMONIC_MINOR = {
	1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1
};
#pragma endregion

#pragma region Note

struct Note {
	KeyInt key;
	NoteTime time, duration;
	NoteVelInt velocity;

	// Key index relative to our octave
	KeyInt GetOctaveKey() const { return key % KEYS_PER_OCTAVE; }

	// Octave index
	KeyInt GetOctave() const { return key / KEYS_PER_OCTAVE; }

	bool IsInKey(const KeyNoteMask& mask, int offset) const {
		return mask[(key + offset) % KEYS_PER_OCTAVE];
	}

	bool IsBlackKey() const {
		return !IsInKey(KEYMASK_MAJOR, 0);
	}

	bool IsValid() const {
		return 
			(key < KEY_AMOUNT) // Key is out-of-bounds (very bad)
			&& NoteTime(time + duration) > time; // Note duration is so long that it will overflow NoteTime integer
	}
};
#pragma endregion
