// Note type and related types
#pragma once
#include "../../Framework.h"
#include "../../Types/Vec.h"
#include "../../Types/Color.h"

#include "MusicTheoryDef.h"

#pragma region Note

struct Note {
	KeyInt key;
	NoteTime time, duration;
	NoteVelInt velocity;

	// Key index relative to our octave
	KeyInt GetOctaveKey() const { return key % KEYS_PER_OCTAVE; }

	// Octave index
	KeyInt GetOctave() const { return key / KEYS_PER_OCTAVE; }

	bool IsInKey(const KeyMasks::KeyNoteMask& mask, int offset) const {
		return mask[(key + offset) % KEYS_PER_OCTAVE];
	}

	bool IsBlackKey() const {
		return !IsInKey(KeyMasks::MAJOR, 0);
	}

	bool IsValid() const {
		return 
			(key < KEY_AMOUNT) // Key is out-of-bounds (very bad)
			&& NoteTime(time + duration) > time; // Note duration is so long that it will overflow NoteTime integer
	}
};
#pragma endregion
