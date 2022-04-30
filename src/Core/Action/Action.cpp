#include "Action.h"

#include "../../Globals.h"

bool Keybind::IsDown() {
	if (!IsValid())
		return false;

	return g_KeyboardState[this->key];
}
