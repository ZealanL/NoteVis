#pragma once
#include "Renderer.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"

namespace Renderer {
	ImDrawList* GetTargetDrawList();

	// Returns true if succeeded
	bool Init();

	void Shutdown();

	void BeginFrame();
	void EndFrame();
}