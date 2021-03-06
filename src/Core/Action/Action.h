#pragma once
#include "../../Framework.h"

struct Keybind {
	SDL_Keycode key;
	byte flags;

	Keybind() {
		this->key = 0;
		this->flags = 0;
	}

	Keybind(SDL_Keycode key, byte flags = 0) {
		this->key = key;
		this->flags = flags;
	}

	bool IsValid() {
		return key > 0;
	}

	bool HasFlag(byte flag) {
		return flags & flag;
	}

	bool IsDown();

	string ToString();
};

// Bindable action that executes something when pressed (save, create note, delete note, etc.)
class Action {
private:
	// Private to prevent execution without going through Execute() execution wrapper
	std::function<void()> fnOnExecute;
public:
	string name, menuName;
	Keybind bind;
	bool undoable; // Can this action be undo'd

	Action(string name, string menuName, std::function<void()> fnOnExecute, Keybind bind, bool undoable) {
		this->name = name;
		this->menuName = menuName;
		this->fnOnExecute = fnOnExecute;
		this->bind = bind;
		this->undoable = undoable;
	}

	void Execute() {
		DLOG("Action::Execute(): {}", name);
		fnOnExecute();
	}
};

// Bindable action that can be held down
class HoldAction {
public:
	string name;
	Keybind bind;

	HoldAction(string name, Keybind bind) {
		this->name = name;
		this->bind = bind;
	}

	bool IsActive() {
		return bind.IsDown();
	}
};

// Keybind flags
enum : byte {
	KBFLAG_CTRL = (1 << 0),
	KBFLAG_ALT = (1 << 1),
	KBFLAG_SHIFT = (1 << 2),

	FLAG_COUNT
};