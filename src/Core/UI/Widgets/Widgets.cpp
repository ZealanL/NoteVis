#include "Widgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../../imgui/imgui_internal.h"

float GetAreaWidth() {
	return ImGui::GetWindowContentRegionMax().x - ImGui::GetCursorPosX();
}

bool Widgets::Button(string label) {
	return ImGui::Button(label.c_str(), ImVec2(GetAreaWidth(), 0));
}

bool Widgets::Checkbox(string label, bool& value) {
	return ImGui::Checkbox(label.c_str(), &value);
}

bool SliderBase(string label, void* value, void* min, void* max, string valueDisplayStr, ImGuiDataType dataType) {

	ImGui::TextUnformatted((label + ":").c_str());

	{ // Draw value text aligned to the right
		ImVec2 valueTextSize = ImGui::CalcTextSize(valueDisplayStr.c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - valueTextSize.x);
		ImGui::TextUnformatted(valueDisplayStr.c_str());
	}

	ImGui::SetNextItemWidth(GetAreaWidth());

	// Double frame padding to increase slider height
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImGui::GetStyle().FramePadding * 2);
	bool result = ImGui::SliderScalar(label.c_str(), dataType, value, min, max, "", ImGuiSliderFlags_NVCustomSlider);
	ImGui::PopStyleVar();
	return result;
}

bool Widgets::SliderInt(string label, int& value, int min, int max, string valueDisplayFormat) {
	return SliderBase(label, &value, &min, &max, FMT(valueDisplayFormat, value), ImGuiDataType_U32);
}

bool Widgets::SliderFloat(string label, float& value, float min, float max, string valueDisplayFormat) {
	return SliderBase(label, &value, &min, &max, FMT(valueDisplayFormat, value), ImGuiDataType_Float);
}

bool Widgets::ColorPicker(string label, Color& value, bool pickAlpha) {
	float tempColFloats[4] = { value.r / 255.f, value.g / 255.f, value.b / 255.f, value.a / 255.f };
	
	int flags = pickAlpha ? (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar) : 0;
	bool result = ImGui::ColorPicker4(label.c_str(), tempColFloats, flags);

	if (result) {
		value.r = tempColFloats[0] * 255;
		value.g = tempColFloats[1] * 255;
		value.b = tempColFloats[2] * 255;

		if (pickAlpha)
			value.a = tempColFloats[3] * 255;
	}

	return result;
}

bool Widgets::Dropdown(string label, int& choice, vector<string> options, bool multiselect) {
	
	ImGui::TextUnformatted((label + ":").c_str());
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x / 3);

	string previewValue;
	if (multiselect) {
		int amountSelected = 0;
		for (int i = 0; i < options.size(); i++) {
			if (choice & (1 << i)) {
				if (!previewValue.empty())
					previewValue += ", ";
				
				previewValue += options[i];
				amountSelected++;
			}
		}

		previewValue = FMT("[{}/{}] ", amountSelected, options.size()) + previewValue;

		if (amountSelected == 0)
			previewValue += "(None)";
		
	} else {
		previewValue = options[choice];
	}

	ImGui::SetNextItemWidth(GetAreaWidth());

	bool result;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImGui::GetStyle().ItemInnerSpacing);
	if (ImGui::BeginCombo(("##" + label).c_str(), previewValue.c_str())) {
		for (int i = 0; i < options.size(); i++) {
			bool selected = multiselect ? (choice & (1 << i)) : (choice == i);
			
			if (ImGui::Selectable(options[i].c_str(), &selected)) {
				if (multiselect) {
					choice ^= (1 << i);
				} else {
					choice = i;
				}
			}
		}

		ImGui::EndCombo();
		result = true;
	} else {
		result = true;
	}
	ImGui::PopStyleVar();
	return result;
}