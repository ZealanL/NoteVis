#include "Action.h"

#include "../../Globals.h"
#include "../Core.h"

bool Keybind::IsDown() {
	if (!IsValid())
		return false;

	return Core::GetCurrentKeybindFlags() == this->flags && g_KeyboardState[SDL_GetScancodeFromKey(this->key)];
}

string Keybind::ToString() {
	if (IsValid()) {
		string result;
		if (flags & KBFLAG_CTRL)	result += "CTRL + ";
		if (flags & KBFLAG_ALT)		result += "ALT + ";
		if (flags & KBFLAG_SHIFT)	result += "SHIFT + ";

		result += SDL_GetKeyName(key);

		return result;
	} else {
		return string();
	}
}
