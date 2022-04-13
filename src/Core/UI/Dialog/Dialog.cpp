#include "Dialog.h"

int Dialog::RenderGetResult() {

	auto io = ImGui::GetIO();
	ImGui::SetNextWindowPos(
		ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
		ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGui::Begin(title.c_str(), NULL,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse
	);

	ImGui::Text(message.c_str());

	for (int i = 0; i < options.size; i++) {
		ImGui::Button(options[i].c_str());
		if (i < options.size - 1) {
			ImGui::SameLine();
		}
	}

	ImGui::End();

	return -1;
}