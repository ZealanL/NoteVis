#pragma once
#include "../../Types/Color.h"
#include "NoteTypes.h"

// This header file is responsible for retrieving the colors I chose for each note of the octave
// I chose these colors very carefully, and thus they are hardcoded into a static array
namespace NoteColors {
	inline Color GetKeyColor(KeyInt key) {
		static Color KEY_COLORS[] = {
			Color(255, 255,   0), Color(255, 255,   0),	// C
			Color(200,  80, 255), Color(200,  80, 255),	// D
			Color( 47, 255,   0),						// E
			Color(  0, 179, 255), Color(  0, 179, 255),	// F
			Color(255, 136,   0), Color(255, 136,   0),	// G
			Color(255,   0, 122), Color(255,   0, 122),	// A
			Color(169, 252, 251),						// B
		};

		SASSERT(ARRAYSIZE(KEY_COLORS) == KEYS_PER_OCTAVE);

		return KEY_COLORS[key % ARRAYSIZE(KEY_COLORS)];
	}
}