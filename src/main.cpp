#include "Framework.h"
#include "Core/Core.h"
#include "Render/Renderer.h"
#include "Core/UI/UI.h"

// NOTE: Mostly from https://github.com/ocornut/imgui/blob/master/examples/example_sdl_opengl3/main.cpp

void RunMainLoop() {
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			g_KeyboardState = SDL_GetKeyboardState(NULL);
			Core::ProcessEvent(event);
		}

		Renderer::BeginFrame();
		Core::OnRender();
		Renderer::EndFrame();
	}
}

int _main() {
#if defined(_DEBUG) and defined(PLAT_WINDOWS)
	// In debug mode, attach a debug console
	AllocConsole();

	// Set stdout stream to print there
	FILE* outStreamFilePtr = NULL;
	auto error = freopen_s(&outStreamFilePtr, "CONOUT$", "w", stdout);
#endif

	DLOG("Starting " PROGRAM_NAME " " PROGRAM_VERSION);

	// Initialize
	Renderer::Init();
	UI::InitImGuiTheme();

	RunMainLoop();

	// Shutdown
	Renderer::Shutdown();

	return EXITCODE_GOOD;
}

#ifdef PLAT_WINDOWS
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