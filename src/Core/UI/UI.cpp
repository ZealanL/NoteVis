#include "UI.h"

#include "../../Globals.h"
#include "../../Render/Renderer.h"
#include "MenuBar/MenuBar.h"

void UI::InitImGuiTheme() {
	auto& s = ImGui::GetStyle();

	{ // Sizes
		s.WindowPadding =		{ 6, 6 };
		s.FramePadding =		{ 4, 2 };
		s.CellPadding =			{ 2, 2 };
		s.ItemSpacing =			{ 8, 4 };
		s.ItemInnerSpacing =	{ 4, 4 };
		s.TouchExtraPadding =	{ 0, 0 };
		s.IndentSpacing =		20.f;
		s.ScrollbarSize =		16.f;
		s.GrabMinSize =			1.f; // Not using grabs anyway

		// Everything should have just 1px of rounding
		s.ChildRounding = 
			s.FrameRounding = 
			s.ScrollbarRounding = 
			s.GrabRounding = 
			s.PopupRounding = 
			s.TabRounding = 
			s.WindowRounding = 1;
	}
	
	{ // Colors
		ImVec4* c = s.Colors;	
		c[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.94f);
		c[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);
		c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_FrameBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.54f);
		c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.05f, 0.05f, 0.21f);
		c[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.67f);
		c[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		c[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		c[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 0.40f);
		c[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		c[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		c[ImGuiCol_Header] = ImVec4(0.36f, 0.36f, 0.36f, 0.31f);
		c[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
		c[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		c[ImGuiCol_Separator] = ImVec4(0.49f, 0.49f, 0.49f, 0.50f);
		c[ImGuiCol_SeparatorHovered] = ImVec4(0.64f, 0.64f, 0.64f, 0.78f);
		c[ImGuiCol_SeparatorActive] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
		c[ImGuiCol_ResizeGrip] = ImVec4(0.63f, 0.63f, 0.63f, 0.20f);
		c[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.60f, 0.60f, 0.67f);
		c[ImGuiCol_ResizeGripActive] = ImVec4(0.88f, 0.88f, 0.88f, 0.95f);
		c[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 0.86f);
		c[ImGuiCol_TabActive] = ImVec4(0.98f, 0.03f, 0.03f, 1.00f);
		c[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 0.97f);
		c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.14f, 0.14f, 1.00f);
		c[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
		c[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	}
}

// Semi-global var
// TODO: Not threadsafe!
stack<Dialog> currentDialogs;

void UI::SetDialog(const Dialog& dialog) {
	currentDialogs.push(dialog);
}

void DrawMenu() {
	MenuBar::Draw();
}

void UI::OnRender() {
	if (!currentDialogs.empty()) {
		auto& currentDialog = currentDialogs.top();

		int choice = currentDialog.RenderGetResult();

		if (choice != Dialog::CHOICE_INVALID) {
			auto itr = currentDialog.executeOnChoiceFuncs.find(choice);
			if (itr != currentDialog.executeOnChoiceFuncs.end())
				itr->second();

			currentDialogs.pop();
		}
	}

	DrawMenu();

	g_WasImGuiMenuActive = ImGui::IsAnyItemFocused() | ImGui::IsAnyItemHovered();
}

bool UI::PreventInput() {
	return !currentDialogs.empty();
}