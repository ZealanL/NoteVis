#pragma once
#include "../../Framework.h"

// An RGBA color
struct Color {
	BYTE r, g, b, a;

	Color(BYTE r = 0, BYTE g = 0, BYTE b = 0, BYTE a = 255) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	// Add a brightness to all RGB values
	// NOTE: Negative brightness is permitted
	Color LinearBrighten(int amount) {
		return Color(
			std::clamp(r + amount, 0, 255),
			std::clamp(g + amount, 0, 255),
			std::clamp(b + amount, 0, 255),
			a
		);
	}

	// Multiply all RGB values by a brightness ratio
	Color RatioBrighten(float ratio) {
		return Color(
			std::clamp(r * ratio, 0.f, 255.f),
			std::clamp(g * ratio, 0.f, 255.f),
			std::clamp(b * ratio, 0.f, 255.f),
			a
		);
	}
};

// Global colors
#define COL_WHITE		Color(255, 255, 255)
#define COL_BLACK		Color(  0,   0,   0)
#define COL_GRAY		Color(100, 100, 100)
#define COL_DARK_GRAY	Color( 50,  50,  50)
#define COL_RED			Color(255,   0,   0)
#define COL_GREEN		Color(  0, 255,   0)
#define COL_BLUE		Color(  0, 100, 255)

// Color as HSVA ([hue, saturation, value, alpha] aka normal HSV with alpha byte)
// Ideal for things like color pickers
// Conversion logic/math from https://gist.github.com/mjackson/5311256
// I would use ImGui utility functions but I might move off of ImGui at some future point
struct HSVColor {
	float h = 0, s = 1, v = 1;
	BYTE alpha = 255;

	HSVColor() {}

	HSVColor(float hue) {
		this->h = hue;
	}

	HSVColor(const Color& color) {
		float r = color.r / 255;
		float g = color.g / 255;
		float b = color.b / 255;

		float max = MAX3(r, g, b), min = MIN3(r, g, b);
		h, s, v = max;

		float d = max - min;
		s = max == 0 ? 0 : d / max;

		if (max == min) {
			h = 0; // achromatic
		} else {
			if (max == r) {
				h = (g - b) / d + (g < b ? 6 : 0);
			} else if (max == b) {
				h = (b - r) / d + 2;
			} else {
				h = (r - g) / d + 4;
			}

			h /= 6;
		}

		this->alpha = color.a;
	}

	operator Color() {
		return GetColor();
	}

	Color GetColor() {
		float r, g, b;

		int i = floor(h * 6);
		float f = h * 6 - i;
		float p = v * (1 - s);
		float q = v * (1 - f * s);
		float t = v * (1 - (1 - f) * s);

		switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
		}

		return Color(r * 255.f, g * 255.f, b * 255.f, alpha);
	}
};