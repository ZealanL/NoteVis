#include "UI.h"

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
		c[ImGuiCol_Text] = 					ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
		c[ImGuiCol_TextDisabled] = 			ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		c[ImGuiCol_WindowBg] = 				ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
		c[ImGuiCol_ChildBg] = 				ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_PopupBg] = 				ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
		c[ImGuiCol_Border] = 				ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		c[ImGuiCol_BorderShadow] = 			ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_FrameBg] = 				ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
		c[ImGuiCol_FrameBgHovered] =		ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
		c[ImGuiCol_FrameBgActive] = 		ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
		c[ImGuiCol_TitleBg] = 				ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
		c[ImGuiCol_TitleBgActive] = 		ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
		c[ImGuiCol_TitleBgCollapsed] = 		ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
		c[ImGuiCol_MenuBarBg] = 			ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
		c[ImGuiCol_ScrollbarBg] = 			ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
		c[ImGuiCol_ScrollbarGrab] = 		ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
		c[ImGuiCol_ScrollbarGrabHovered] = 	ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
		c[ImGuiCol_ScrollbarGrabActive] = 	ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
		c[ImGuiCol_CheckMark] = 			ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		c[ImGuiCol_SliderGrab] = 			ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		c[ImGuiCol_SliderGrabActive] = 		ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
		c[ImGuiCol_Button] = 				ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
		c[ImGuiCol_ButtonHovered] = 		ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
		c[ImGuiCol_ButtonActive] = 			ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
		c[ImGuiCol_Header] = 				ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
		c[ImGuiCol_HeaderHovered] = 		ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
		c[ImGuiCol_HeaderActive] = 			ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
		c[ImGuiCol_Separator] = 			ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		c[ImGuiCol_SeparatorHovered] = 		ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
		c[ImGuiCol_SeparatorActive] = 		ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
		c[ImGuiCol_ResizeGrip] = 			ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		c[ImGuiCol_ResizeGripHovered] = 	ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
		c[ImGuiCol_ResizeGripActive] = 		ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
		c[ImGuiCol_Tab] = 					ImVec4(0.34f, 0.34f, 0.68f, 0.79f);
		c[ImGuiCol_TabHovered] = 			ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
		c[ImGuiCol_TabActive] = 			ImVec4(0.40f, 0.40f, 0.73f, 0.84f);
		c[ImGuiCol_TabUnfocused] = 			ImVec4(0.28f, 0.28f, 0.57f, 0.82f);
		c[ImGuiCol_TabUnfocusedActive] = 	ImVec4(0.35f, 0.35f, 0.65f, 0.84f);
		c[ImGuiCol_PlotLines] = 			ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		c[ImGuiCol_PlotLinesHovered] = 		ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		c[ImGuiCol_PlotHistogram] = 		ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		c[ImGuiCol_PlotHistogramHovered] = 	ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		c[ImGuiCol_TableHeaderBg] = 		ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
		c[ImGuiCol_TableBorderStrong] = 	ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
		c[ImGuiCol_TableBorderLight] = 		ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
		c[ImGuiCol_TableRowBg] = 			ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		c[ImGuiCol_TableRowBgAlt] = 		ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
		c[ImGuiCol_TextSelectedBg] = 		ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		c[ImGuiCol_DragDropTarget] = 		ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		c[ImGuiCol_NavHighlight] = 			ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
		c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		c[ImGuiCol_NavWindowingDimBg] = 	ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		c[ImGuiCol_ModalWindowDimBg] = 		ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
}