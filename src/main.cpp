#include "Framework.h"
#include "Core/Core.h"
#include "Render/Renderer.h"
#include "Core/UI/UI.h"

// NOTE: Mostly from https://github.com/ocornut/imgui/blob/master/examples/example_sdl_opengl3/main.cpp

void RunMainLoop() {
	while (true) {

		// Update keyboard/mouse state prior to any event parsing
		// This is so that changes to keyboard/mouse state do not preceed the processing of their corresponding events
		//	(i.e. g_MouseState[x] won't be set to true before SDL_MOUSEBUTTONDOWN is processed)
		// TODO: SDL already has a way to do this (SDL_PumpEvents ?), implement that
		memcpy(g_KeyboardState, SDL_GetKeyboardState(NULL), sizeof(g_KeyboardState));
		auto mouseStateInt = SDL_GetMouseState(NULL, NULL);
		for (int i = 0; i < sizeof(g_MouseState); i++)
			g_MouseState[i] = mouseStateInt & i;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			
			// Manually update states with each event
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
				g_MouseState[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				g_MouseState[event.button.button] = false;
				break;
			case SDL_KEYDOWN:
				g_KeyboardState[event.key.keysym.scancode] = true;
				break;
			case SDL_KEYUP:
				g_KeyboardState[event.key.keysym.scancode] = false;
			}
		}

		Renderer::BeginFrame();

		Core::OnRender();
		UI::OnRender();

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