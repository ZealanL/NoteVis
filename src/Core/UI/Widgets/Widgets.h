#include "../../../Types/Vec.h"
#include "../../../Types/Color.h"

// General UI widgets, extending off of ImGui
namespace Widgets {
	bool Button(string label);
	bool Checkbox(string label, bool& value);
	bool SliderInt(string label, int& value, int min, int max, string valueDisplayFormat = "{}");
	bool SliderFloat(string label, float& value, float min, float max, string valueDisplayFormat = "{:.3f}");
	bool ColorPicker(string label, Color& value, bool pickAlpha = false);

	// If multiselect is set to true, output "choice" variable will be used as bitflags instead of index
	bool Dropdown(string label, int& choice, vector<string> options, bool multiselect = false);
}