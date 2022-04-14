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
	VelInt velocity;

	// Key index relative to our octave
	KeyInt GetOctaveKey() const { return key % KEYS_PER_OCTAVE; }

	// Octave index
	KeyInt GetOctave() const { return key / KEYS_PER_OCTAVE; }

	bool IsInKey(const KeyMask::Mask& mask, int offset) const {
		return mask[(key + offset) % KEYS_PER_OCTAVE];
	}

	bool IsBlackKey() const {
		return !IsInKey(KeyMask::MAJOR, 0);
	}

	bool IsValid() const {
		return 
			(key < KEY_AMOUNT) // Key is not out-of-bounds (that would be very bad)
			&& time >= 0; // Note time is valid
	}
};
#pragma endregion
