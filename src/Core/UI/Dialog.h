#pragma once
#include "../Core.h"

struct Dialog {
	string title, message;
	StaticList<string> options;

	Dialog(string title, string message, StaticList<string> options)
		: title(title), message(message), options(options) {}
	
	// Returns -1 if nothing was selected
	int RenderGetResult();
};