#include "Draw.h"

// Utility macros for converting to ImGui types
#define IMV(vec) ImVec2(vec.x, vec.y)
#define IMC(col) ((col.a & 0xff) << 24) + ((col.b & 0xff) << 16) + ((col.g & 0xff) << 8) + ((col.r & 0xff))

#define DL Renderer::GetTargetDrawList()

void Draw::Line(Vec start, Vec end, Color color, float thickness) {
	DL->AddLine(IMV(start), IMV(end), IMC(color), thickness);
}

void Draw::PixelPerfectLine(Vec start, Vec end, Color color, float thickness) {
	DL->AddLine(IMV(start.Rounded()), IMV(end.Rounded()), IMC(color), thickness);
}

void Draw::Rect(Vec start, Vec end, Color color, float rounding) {
	DL->AddRectFilled(IMV(start), IMV(end), IMC(color), rounding);
}
void Draw::ORect(Vec start, Vec end, Color color, float rounding) {
	DL->AddRect(IMV(start), IMV(end), IMC(color), rounding);
}

void Draw::Rect(Area area, Color color, float rounding) {
	Draw::Rect(area.min, area.max, color, rounding);
}
void Draw::ORect(Area area, Color color, float rounding) {
	Draw::ORect(area.min, area.max, color, rounding);
}

void Draw::Circle(Vec center, Color color, float radius) {
	int numPoints = MAX(3, sqrtf(radius) * 8);
	DL->AddCircleFilled(IMV(center), radius, IMC(color), numPoints);
}

void Draw::OCircle(Vec center, Color color, float radius, float thickness) {
	int numPoints = MAX(3, sqrtf(radius) * 8);
	DL->AddCircle(IMV(center), radius, IMC(color), numPoints);
}

void Draw::ConvexPoly(vector<Vec> points, Color color) {
	if (!points.empty())
		DL->AddConvexPolyFilled((ImVec2*)points.begin()._Ptr, points.size(), IMC(color));
}

void Draw::OPoly(vector<Vec> points, Color color, float thickness) {
	if (!points.empty())
		DL->AddPolyline((ImVec2*)points.begin()._Ptr, points.size(), IMC(color), ImDrawListFlags_AntiAliasedLines, thickness);
}

Vec Draw::GetTextSize(string str) {
	float size = ImGui::GetFontSize();
	auto font = ImGui::GetFont();
	return Vec(font->CalcTextSizeA(size, FLT_MAX, 0, str.c_str()));
}

void Draw::Text(string str, Vec pos, Color color, Vec center) {
	if (!str.empty()) {
		float size = ImGui::GetFontSize();
		auto font = ImGui::GetFont();
		Vec realPos = pos + center * font->CalcTextSizeA(size, FLT_MAX, 0, str.c_str());
		DL->AddText(font, size, IMV(realPos), IMC(color), str.c_str());
	}
}