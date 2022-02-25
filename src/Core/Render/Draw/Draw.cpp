#include "Draw.h"

// Utility macros for converting to ImGui types
#define IMV(vec) ImVec2(vec.x, vec.y)
#define IMC(col) ((col.a & 0xff) << 24) + ((col.b & 0xff) << 16) + ((col.g & 0xff) << 8) + ((col.r & 0xff))

void Draw::Line(Vec start, Vec end, Color color, float thickness) {
	Renderer::GetTargetDrawList()->AddLine(IMV(start), IMV(end), IMC(color), thickness);
}

void Draw::PixelPerfectLine(Vec start, Vec end, Color color, float thickness) {
	Renderer::GetTargetDrawList()->AddLine(IMV(start.Rounded()), IMV(end.Rounded()), IMC(color), thickness);
}

void Draw::Rect(Vec start, Vec end, Color color, float rounding) {
	Renderer::GetTargetDrawList()->AddRectFilled(IMV(start), IMV(end), IMC(color), rounding);
}
void Draw::ORect(Vec start, Vec end, Color color, float rounding) {
	Renderer::GetTargetDrawList()->AddRect(IMV(start), IMV(end), IMC(color), rounding);
}

void Draw::Rect(Area area, Color color, float rounding) {
	Draw::Rect(area.min, area.max, color, rounding);
}
void Draw::ORect(Area area, Color color, float rounding) {
	Draw::ORect(area.min, area.max, color, rounding);
}

void Draw::Circle(Vec center, Color color, float radius) {
	int numPoints = MAX(3, sqrtf(radius) * 8);
	Renderer::GetTargetDrawList()->AddCircleFilled(IMV(center), radius, IMC(color), numPoints);
}

void Draw::OCircle(Vec center, Color color, float radius, float thickness) {
	int numPoints = MAX(3, sqrtf(radius) * 8);
	Renderer::GetTargetDrawList()->AddCircle(IMV(center), radius, IMC(color), numPoints);
}

void Draw::ConvexPoly(vector<Vec> center, Color color) {

	if (!center.empty())
		Renderer::GetTargetDrawList()->AddConvexPolyFilled((ImVec2*)center.begin()._Ptr, center.size(), IMC(color));
}

void Draw::OPoly(vector<Vec> center, Color color, float thickness) {
	if (!center.empty())
		Renderer::GetTargetDrawList()->AddPolyline((ImVec2*)center.begin()._Ptr, center.size(), IMC(color), ImDrawListFlags_AntiAliasedLines, thickness);
}