#pragma once
#include "../Action.h"

class ActionDefineSystem {
public:
	unordered_map<string, Action*> actions;
	unordered_map<string, HoldAction*> holdActions;

	void Init();
};