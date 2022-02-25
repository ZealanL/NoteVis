#include "Renderer.h"
#include "../Core.h"

// Draw list to be used for draw functions
ImDrawList* targetDrawList = NULL;
ImDrawList* Renderer::GetTargetDrawList() {
	return targetDrawList;
}

bool Renderer::Init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		ERRORCLOSE("Failed to initialize SDL (SDL error: " << SDL_GetError() << ")");
		return EXITCODE_FAIL;
	}

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

	// Size is temporary
	g_SDL_Window = SDL_CreateWindow("NoteVis " NV_VERSION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1270, 512, window_flags);
	SDL_MaximizeWindow(g_SDL_Window);

	g_SDL_GLContext = SDL_GL_CreateContext(g_SDL_Window);
	SDL_GL_MakeCurrent(g_SDL_Window, g_SDL_GLContext);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	 // Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(g_SDL_Window, g_SDL_GLContext);
	const char* glslVersion = "#version 130";
	ImGui_ImplOpenGL3_Init(glslVersion);
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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	targetDrawList = ImGui::GetBackgroundDrawList();
}

void Renderer::EndFrame() {
	Vec windowSize = Core::GetWindowSize();
	glViewport(0, 0, windowSize.x, windowSize.y);
	glClearColor(0.1f, 0.1f, 0.1f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(g_SDL_Window);
}