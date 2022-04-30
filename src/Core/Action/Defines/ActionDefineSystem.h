#pragma once
#include "../Action.h"

class ActionDefineSystem {
public:
	vector<Action*> actions;
	vector<HoldAction*> holdActions;

	// Hold actions go here
	HoldAction* ha_ChangeNoteVels;

	void Init();
};