#pragma once
#include "../Renderer.h"

// General drawing functions
namespace Draw {
	// O is short for outline

	void Line(Vec start, Vec end, Color color, float thickness = 1.f);
	void PixelPerfectLine(Vec start, Vec end, Color color, float thickness = 1.f);

	void Rect(Vec start, Vec end, Color color, float rounding = 0.f);
	void ORect(Vec start, Vec end, Color color, float rounding = 0.f);

	void Rect(Area area, Color color, float rounding = 0.f);
	void ORect(Area area, Color color, float rounding = 0.f);

	void Circle(Vec center, Color color, float radius);
	void OCircle(Vec center, Color color, float radius, float thickness = 1.f);

	void ConvexPoly(vector<Vec> points, Color color);
	void OPoly(vector<Vec> points, Color color, float thickness = 1.f);

	Vec GetTextSize(string str, float wrapWidth = 0);

	void Text(string str, Vec pos, Color color = { 255, 255, 255 }, Vec center = { 0.5f, 0.5f }, byte shadowAlpha = 0);
    void TextInArea(string str, Area wrapArea, Color color = { 255, 255, 255 }, bool centerInArea = false);
    void StartClip(Area area);
    void EndClip();
}