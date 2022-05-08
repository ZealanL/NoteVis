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
			(key < KEY_AMOUNT)	// Key is not out-of-bounds (that would be very bad)
			&& velocity > 0		// Velocity is valid
			&& time >= 0;		// Note time is valid
	}

	bool operator==(const Note& other) {
		return time == other.time && key == other.key 
			&& duration == other.duration && velocity == other.velocity;
	}

	int Compare(const Note& other) {
		if (time != other.time)
			return time < other.time;
		if (key != other.key)
			return key < other.key;
		if (duration != other.duration)
			return duration < other.duration;
		if (velocity != other.velocity)
			return velocity < other.velocity;

		return false; // Exact same
	}
};
#pragma endregion
