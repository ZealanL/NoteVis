#pragma once
#include "../../../Types/StaticList.h"

struct Dialog {
	typedef std::function<void()> ExecuteFunc;

	enum {
		CHOICE_INVALID = -1
	};

	string title, message;
	StaticList<string> options;

	// Functions that will execute when a certain option index is chosen
	map<int, ExecuteFunc> executeOnChoiceFuncs;

	Dialog(string title, string message, StaticList<string> options = { "OK" })
		: title(title), message(message), options(options) {}

	// Returns CHOICE_INVALID if nothing was selected
	// It is up to the caller to handle executeOnChoice
	int RenderGetResult();
};