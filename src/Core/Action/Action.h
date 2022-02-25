#pragma once
#include "../../Framework.h"

// Bindable action (save, create note, delete note, etc.)
class Action {
private:
	// Private to prevent execution without going through Execute() execution wrapper
	std::function<void()> fnOnExecute;
public:

	struct Keybind {
		SDL_Keycode key;
		BYTE flags;

		Keybind() {
			this->key = 0;
			this->flags = 0;
		}

		Keybind(SDL_Keycode key, BYTE flags = 0) {
			this->key = key;
			this->flags = flags;
		}

		bool IsValid() {
			return key > 0;
		}

		bool HasFlag(BYTE flag) {
			return flags & flag;
		}
	};

	string name;
	Keybind bind;

	Action(string name, std::function<void()> fnOnExecute, Keybind bind) {
		this->name = name;
		this->fnOnExecute = fnOnExecute;
		this->bind = bind;
	}

	void Execute() {
		DLOG("Action::Execute(): \"" << name << '"');
		fnOnExecute();
	}
};

// Keybind flags
enum : BYTE {
	KBFLAG_CTRL = (1 << 0),
	KBFLAG_ALT = (1 << 1),
	KBFLAG_SHIFT = (1 << 2),

	FLAG_COUNT
};