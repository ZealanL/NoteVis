#pragma once
#include "Renderer.h"
#include "../Types/Vec.h"
#include "../Types/Color.h"

struct DevRenderStats {
	int fps = 0, curFramesRendered = 0;

	double averageDrawTimePerFrame = 0;

	double timeSpendDrawing = 0;
	double lastBeginFrameTime = 0, lastEndFrameTime = 0;
	double lastBeginDrawTime = 0, lastEndDrawTime = 0;
};

namespace Renderer {

	inline DevRenderStats devRenderStats;

	ImDrawList* GetTargetDrawList();

	// Returns true if succeeded
	bool Init();

	void Shutdown();

	void BeginFrame();
	void EndFrame();

	Vec GetWindowSize();
}