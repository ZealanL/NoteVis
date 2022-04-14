#pragma once

#include "../../Framework.h"

// A low-level header for music theory related definitions such as chord masks

#define KEYS_PER_OCTAVE 12

#define KEY_AMOUNT (88 + 9) // Most pianos have 88, I have extended it down to a very low C such that an index 0 key is on C
#define KEY_AMOUNT_SUB1 (KEY_AMOUNT - 1)
#define KEY_AMOUNT_SUB1_F ((float)(KEY_AMOUNT - 1))

// Represnted in milliseconds at 60 BPM
// Originally this was unsigned, but I changed it to allow for proper negative value correction
typedef int32 NoteTime;

// NoteTime each beat (60 BPM ms = 1000 NOTETIME_PER_BEAT)
#define NOTETIME_PER_BEAT 1000

// Integer representing key index
// Always should be < TOTAL_KEYS
typedef uint8 KeyInt;

// Note velocity, from 0 to 255
typedef uint8 VelInt;

ENUM_SCOPE(KeyIndex,
	C, Cs, Db = Cs, D, Ds, Eb = Ds, E, F, Fs, Gb = Fs, G, Gs, Ab = Gs, A, As, Bb = As, B,

	COUNT
);

SASSERT(KeyIndex::COUNT == KEYS_PER_OCTAVE);

namespace KeyMask {
	typedef const bool Mask[KEYS_PER_OCTAVE];

	constexpr Mask MAJOR = {
		1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1
	};

	constexpr Mask MINOR = {
		1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0
	};

	constexpr Mask HARMONIC_MINOR = {
		1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1
	};
}

namespace ChordDef {
	struct KeySet {
		string name;

		// NOTE: Should always start on C
		vector<KeyInt> relKeys;
		
		KeySet(string name, std::initializer_list<KeyInt> relKeysInit) {
			ASSERT(relKeysInit.size() > 0);
			ASSERT(*(relKeysInit.begin()) == KeyIndex::C);

			this->name = name;
			this->relKeys = relKeysInit;
		}
	};

#define MAKE(name, ...) const KeySet SET_##name = KeySet(#name, ##__VA_ARGS__)
	using namespace KeyIndex;

	MAKE(Maj, {C, E, G});
	MAKE(Min, {C, Eb, G});

	MAKE(Dom7, { C, E, G, Bb });
	MAKE(Maj7, { C, E, G, B });
	MAKE(Min7, { C, Eb, G, Bb });

	MAKE(Dim7, { C, E, Gs, As });
	MAKE(MinMaj7, { C, Eb, G, B });

#undef MAKE
}
