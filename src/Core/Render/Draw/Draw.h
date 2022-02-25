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
}