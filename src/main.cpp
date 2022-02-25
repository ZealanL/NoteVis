#include "Framework.h"
#include "Core/Core.h"
#include "Core/Render/Renderer.h"

// NOTE: Mostly from https://github.com/ocornut/imgui/blob/master/examples/example_sdl_opengl3/main.cpp

void RunMainLoop() {
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			Core::ProcessEvent(event);
		}

		Renderer::BeginFrame();
		Core::OnRender();
		Renderer::EndFrame();
	}
}

int _main() {
#ifdef _DEBUG
#ifdef NV_WINDOWS
	// In debug mode, attach a debug console
	AllocConsole();

	// Set stdout stream to print there
	FILE* outStreamFilePtr = nullptr;
	auto error = freopen_s(&outStreamFilePtr, "CONOUT$", "w", stdout);
#endif
#endif

	DLOG("Starting NoteVis v" << NV_VERSION);

	Renderer::Init();
	RunMainLoop();
	Renderer::Shutdown();

	
	Sleep(-1);
	return EXITCODE_GOOD;
}

#ifdef NV_WINDOWS
#include <Windows.h>
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	return _main();
}
#endif