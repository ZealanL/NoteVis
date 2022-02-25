#pragma once
#include "../../Framework.h"

// A type representing a 2D floating-point vector
struct Vec {
	float x, y;

	Vec() {
		x = y = 0;
	}

	Vec(float x, float y) { this->x = x; this->y = y; }

	Vec(float both) {
		this->x = both;
		this->y = both;
	}

	float Length() const {
		return sqrtf(x * x + y * y);
	}

	float LengthSqr() const {
		return x * x + y * y;
	}

	float Distance(Vec other) const {
		float dx = x - other.x;
		float dy = y - other.y;
		return sqrtf(dx * dx + dy * dy);
	}

	float DistanceSqr(Vec other) const {
		float dx = x - other.x;
		float dy = y - other.y;
		return dx * dx + dy * dy;
	}

	Vec Rounded() const {
		return Vec(roundf(x), roundf(y));
	}

	// Creation operators
	Vec operator +(Vec a) const {
		return Vec(x + a.x, y + a.y);
	}
	Vec operator -(Vec a) const {
		return Vec(x - a.x, y - a.y);
	}
	Vec operator *(Vec a) const {
		return Vec(x * a.x, y * a.y);
	}
	Vec operator /(Vec a) const {
		return Vec(a.x == 0 ? 0 : x / a.x, a.y == 0 ? 0 : y / a.y);
	}
	Vec operator *(float a) const {
		return Vec(x * a, y * a);
	}
	Vec operator /(float a) const {
		if (a == 0) return Vec();
		return Vec(x / a, y / a);
	}

	// Modifying operators
	Vec& operator+=(const Vec& v) {
		x += v.x; y += v.y; return *this;
	}
	Vec& operator-=(const Vec& v) {
		x -= v.x; y -= v.y; return *this;
	}
	Vec& operator*=(float v) {
		x *= v; y *= v; return *this;
	}
	Vec& operator/=(float v) {
		x /= v; y /= v; return *this;
	}

	bool operator ==(Vec other) const {
		return x == other.x && y == other.y;
	}

	bool operator <(Vec a) const {
		return x < a.x && y < a.y;
	}

	bool operator >(Vec a) const {
		return x > a.x && y > a.y;
	}

	bool operator <=(Vec a) const {
		return x <= a.x && y <= a.y;
	}

	bool operator >=(Vec a) const {
		return x >= a.x && y >= a.y;
	}

	std::string ToString(bool rounded = true) {
		if (rounded) {
			return STR("[ " << x << ", " << y << "]");
		} else {
			return STR("[ " << (int)x << ", " << (int)y << "]");
		}
	}
	
	ImVec2 ToIm() {
		return *(ImVec2*)this;
	}
};

// Required for forced conversion
NV_SASSERT(sizeof(Vec) == sizeof(ImVec2));

// 2D Area made from 2 Vecs, min and max
struct Area {
	Vec min, max;
	Area(Vec a, Vec b) {
		min.x = MIN(a.x, b.x);
		min.y = MIN(a.y, b.y);
		max.x = MAX(a.x, b.x);
		max.y = MAX(a.y, b.y);
	}

	Vec Size() {
		return max - min;
	}

	bool IsValid() {
		return max > min;
	}

	// NOTE: Negative amounts are permitted, however it may cause an invalid area
	Area Expand(float amount) {
		return Area{ min - amount, max + amount };
	}
};