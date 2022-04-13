#pragma once
#include "Dialog/Dialog.h"

namespace UI {
	void InitImGuiTheme();
	
	void OnRender();

	void SetDialog(const Dialog& dialog);

	// Returns true if something should be preventing normal application input (i.e. popup)
	bool PreventInput();
}