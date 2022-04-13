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
	
	static Dialog MakeYesNo(string title, string message, ExecuteFunc onYesFunc = NULL, ExecuteFunc onNoFunc = NULL) {
		Dialog dialog = Dialog(title, message, { "Yes", "No" });
		if (onYesFunc)
			dialog.executeOnChoiceFuncs[0] = onYesFunc;
		if (onNoFunc)
			dialog.executeOnChoiceFuncs[0] = onNoFunc;

		return dialog;
	}

	// Returns CHOICE_INVALID if nothing was selected
	// It is up to the caller to handle executeOnChoice
	int RenderGetResult();
};