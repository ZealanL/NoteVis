#include "Renderer.h"
#include "../Core/Core.h"
#include "Draw/Draw.h"
#include "../Core/NVFileSystem/NVFileSystem.h"
#include "../imgui/imgui_internal.h"

// Draw list to be used for draw functions
ImDrawList* targetDrawList = NULL;
ImDrawList* Renderer::GetTargetDrawList() {
	return targetDrawList;
}

bool Renderer::Init() {
	// NOTE: Mostly from https://github.com/ocornut/imgui/blob/master/examples/example_sdl_opengl3/main.cpp


	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		FW::FatalError("Failed to initialize SDL (SDL error: %s)", SDL_GetError());

	// GL 3.0 + GLSL 130
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	g_SDL_Window = SDL_CreateWindow(PROGRAM_NAME " " PROGRAM_VERSION " " ARCH_LABEL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DEFAULT_WINDOWSIZE_X, DEFAULT_WINDOWSIZE_Y, window_flags);

	g_SDL_GLContext = SDL_GL_CreateContext(g_SDL_Window);
	SDL_GL_MakeCurrent(g_SDL_Window, g_SDL_GLContext);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	 // Set up Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Set up Dear ImGui style
	ImGui::StyleColorsDark();

	// Set up Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(g_SDL_Window, g_SDL_GLContext);
	const char* glslVersion = "#version 130";
	ImGui_ImplOpenGL3_Init(glslVersion);

	// Set up ImGui font
	ByteDataStream fontData;
	wstring fontFilePath;

#ifdef PLAT_WINDOWS
	wchar_t fontFolderPathBuffer[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, fontFolderPathBuffer);

	fontFilePath = fontFolderPathBuffer;
	fontFilePath += L"\\tahoma.ttf";
#endif

	if (!fontFilePath.empty() && NVFileSystem::LoadFile(fontFilePath, fontData)) {
		ImFontConfig fontCfg;
		fontCfg.OversampleH = 4;
		ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.GetBasePointer(), fontData.size(), 16, &fontCfg);
		DLOG("Loaded font \"{}\", file size: {}", string(fontFilePath.begin(), fontFilePath.end()), fontData.size());
		ImGui::GetIO().Fonts->Build();
	} else {
		DLOG("Failed to find font to load, using ImGui default.");
	}

	return true;
}

void Renderer::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(g_SDL_GLContext);
	SDL_DestroyWindow(g_SDL_Window);
	SDL_Quit();
}

void Renderer::BeginFrame() {
	string windowTitle = (PROGRAM_NAME " " PROGRAM_VERSION " " ARCH_LABEL " - ") + NVFileSystem::GetCurrentScoreName();
	if (g_HasUnsavedChanges) {
		windowTitle += '*';
	}
	SDL_SetWindowTitle(g_SDL_Window, windowTitle.c_str());

	devRenderStats.lastBeginFrameTime = CURRENT_TIME;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	targetDrawList = ImGui::GetBackgroundDrawList();

	devRenderStats.lastBeginDrawTime = CURRENT_TIME;
}

void Renderer::EndFrame() {
	if (g_ARG_DevMode) { // Show render stats
		Draw::Text(
			FMT("{} fps\n{}", devRenderStats.fps, FW::TimeDurationToString(devRenderStats.averageDrawTimePerFrame)),
			Vec(GetWindowSize().x - 8, 8 + ImGui::GetFrameHeight()), COL_WHITE, Vec(1, 0), 100
		);
	}

	devRenderStats.lastEndDrawTime = CURRENT_TIME;

	Vec windowSize = GetWindowSize();
	glViewport(0, 0, windowSize.x, windowSize.y);
	glClearColor(0.1f, 0.1f, 0.1f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();
	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(g_SDL_Window);

	devRenderStats.lastEndFrameTime = CURRENT_TIME;
	{ // Update stats

		devRenderStats.curFramesRendered++;
		devRenderStats.timeSpendDrawing += devRenderStats.lastEndDrawTime - devRenderStats.lastBeginDrawTime;
		if (devRenderStats.lastUpdateTime + 1 <= CURRENT_TIME) {
			// A second has passed
			devRenderStats.lastUpdateTime = CURRENT_TIME;

			devRenderStats.fps = devRenderStats.curFramesRendered;
			devRenderStats.averageDrawTimePerFrame = devRenderStats.timeSpendDrawing / (double)devRenderStats.fps;

			// Reset
			devRenderStats.timeSpendDrawing = 0;
			devRenderStats.curFramesRendered = 0;
		}
	}
}

Vec Renderer::GetWindowSize() {
	int w, h;
	SDL_GetWindowSize(g_SDL_Window, &w, &h);
	return Vec(w, h);
}