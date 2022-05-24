#include "Framework.h"
#include "Core/Core.h"
#include "Render/Renderer.h"
#include "Core/UI/UI.h"

void RunMainLoop() {
	while (true) {

		// Update keyboard/mouse state prior to any event parsing
		// This is so that changes to keyboard/mouse state do not preceed the processing of their corresponding events
		//	(i.e. g_MouseState[x] won't be set to true before SDL_MOUSEBUTTONDOWN is processed)
		// TODO: SDL already has a way to do this (SDL_PumpEvents ?), implement that maybe?
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

			ImGui_ImplSDL2_ProcessEvent(&event);
			Core::ProcessEvent(event);		
		}

		Renderer::BeginFrame();

		Core::OnRender();
		UI::OnRender();

		Renderer::EndFrame();
	}
}

int _main(vector<string> args) {
#if defined(_DEBUG) and defined(PLAT_WINDOWS)
	// In debug mode, attach a debug console
	AllocConsole();

	// Set stdout stream to print there
	FILE* outStreamFilePtr = NULL;
	auto error = freopen_s(&outStreamFilePtr, "CONOUT$", "w", stdout);
#endif

	DLOG("Starting " PROGRAM_NAME " " PROGRAM_VERSION ", number of command line args: {}", args.size());
	for (auto arg : args) {
		// TODO: Messy
		size_t spacePos = arg.find(' ');
		if (spacePos == string::npos)
			spacePos = arg.size();

		string argName = arg.substr(0, spacePos);
		string argVal = arg.substr(spacePos);

		if (argName == "-maxhistorymem") {
			try {
				int amount = std::stoi(argVal, NULL, NULL);
				if (amount < 1 || amount > 8000)
					throw std::exception();

				g_ARG_MaxHistoryMemSize = amount;
				DLOG("Set history memory limit to {}mb", amount);
			} catch (std::exception& e) {
				FW::FatalError("Arg \"maxhistorymem\" should have a number from 1 to 8000");
			}

			break;
		}

		if (argName == "-dev") {
			g_ARG_DevMode = true;
			DLOG("Running in DEV MODE!");
			break;
		}

		FW::ShowError("Ignoring unknown command line argument \"{}\"", arg);
	}

	// Initialize
	g_Actions.Init();
	Renderer::Init();
	UI::InitImGuiTheme();

	RunMainLoop();

	// Shutdown
	Renderer::Shutdown();

	return EXITCODE_GOOD;
}

#ifdef PLAT_WINDOWS
#include <Windows.h>
#include <shellapi.h>
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	int argCount;
	LPWSTR* argsPtr = CommandLineToArgvW(GetCommandLineW(), &argCount);

	vector<string> args;
	args.reserve(argCount);

	// Parse command line arguments
	// TODO: Make a clean system to do this
	for (int i = 1; i < argCount; i++) {
		wstring wArg = argsPtr[i];
		string arg = string(wArg.begin(), wArg.end());

		if (!args.empty() && args.back().front() == '-' && arg.front() != '-') {
			// Last argument was prefixed by '-', this one isn't - it must be a subargument
			args.back() += ' ' + arg;
		} else {
			args.push_back(arg);
		}
	}

	return _main(args);
}
#endif