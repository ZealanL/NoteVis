#pragma once
#include "../Framework.h"

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

	Vec(ImVec2 imVec) {
		this->x = imVec.x;
		this->y = imVec.y;
	}

	float Length() const {
		return std::sqrtf(x * x + y * y);
	}

	float LengthSqr() const {
		return x * x + y * y;
	}

	float Distance(Vec other) const {
		float dx = x - other.x;
		float dy = y - other.y;
		return std::sqrtf(dx * dx + dy * dy);
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
		return x < a.x&& y < a.y;
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

	string ToString(bool rounded = true) {
		return FMT(rounded ? "[%f, %f]" : "[%i, %i]", x, y);
	}

	ImVec2 ToImVec() {
		return *(ImVec2*)this;
	}
};

// Required for forced conversion
SASSERT(sizeof(Vec) == sizeof(ImVec2));

// 2D Area made from 2 Vecs, min and max
struct Area {
	Vec min, max;

	Area(Vec a, Vec b) {
		min.x = MIN(a.x, b.x);
		min.y = MIN(a.y, b.y);
		max.x = MAX(a.x, b.x);
		max.y = MAX(a.y, b.y);
	}

	bool IsValid() {
		return (min.x <= max.x && min.y <= max.y);
	}

	bool IsInside(Vec other) {
		return (other.x <= max.x && other.y <= max.y) && (other.x >= min.x && other.y >= max.y);
	}

	bool Overlaps(Area other) {
		return
			(min.x <= other.max.x && min.y <= other.max.y) &&
			(max.x >= other.min.x && max.y >= other.min.y);
	}

	Area Combine(Area other) {
		Vec combinedMin, combinedMax;

		combinedMin.x = MIN(other.min.x, min.x);
		combinedMin.y = MIN(other.min.y, min.y);

		combinedMax.x = MAX(other.max.x, max.x);
		combinedMax.y = MAX(other.max.y, max.y);

		return Area(combinedMin, combinedMax);
	}

	float GetWidth() { return max.x - min.x; }
	float GetHeight() { return max.y - min.y; }
	Vec GetSize() { return Vec(GetWidth(), GetHeight()); }

	Vec GetCenter() {
		return (min + max) / 2;
	}

	Area HDivRel(float beginRatio, float endRatio) {
		assert(beginRatio > endRatio);
		float baseWidth = GetWidth();

		Area newArea = *this;
		newArea.min.x = min.x + beginRatio * baseWidth;
		newArea.max.x = min.x + endRatio * baseWidth;
		return newArea;
	}

	Area VDivRel(float beginRatio, float endRatio) {
		assert(beginRatio > endRatio);
		float baseHeight = GetHeight();

		Area newArea = *this;
		newArea.min.y = min.y + beginRatio * baseHeight;
		newArea.max.y = min.y + endRatio * baseHeight;
		return newArea;
	}

	Area HDiv(float beginWidth, float endWidth) {
		assert(beginWidth > endWidth);
		Area newArea = *this;
		newArea.min.x = min.x + beginWidth;
		newArea.max.x = min.x + endWidth;
		return newArea;
	}

	Area VDiv(float beginHeight, float endHeight) {
		assert(beginHeight > endHeight);
		Area newArea = *this;
		newArea.min.y = min.y + beginHeight;
		newArea.max.y = min.y + endHeight;
		return newArea;
	}

	// Get a horizontal row from the top, down a specific height
	Area Row(float height, bool clip = false) {
		Area newArea = *this;
		newArea.max = newArea.min + (clip ? MIN(GetHeight(), height) : height);
	}

	// Area is expanded outwards by a certain amount (or shrunk if negative)
	Area Expand(float amount) {
		Vec expandVec = Vec(MIN(amount, GetWidth() / 2), MIN(amount, GetHeight() / 2));
		return Area(min - expandVec, max + expandVec);
	}

	// Width or height is clamped such that the area is a square
	Area SquareShrink(bool center = false) {
		float smallestSize = MIN(GetWidth(), GetHeight());

		if (center) {
			auto centerPos = GetCenter();
			auto sizeVec = Vec(smallestSize / 2);
			return Area(centerPos - sizeVec, centerPos + sizeVec);
		} else {
			return Area(min, min + smallestSize);
		}
	}

	Vec GetPosRel(float widthRatio, float heightRatio) {
		return Vec(min.x + GetWidth() * widthRatio, min.y + GetHeight() * heightRatio);
	}

	Vec TopLeft() { return GetPosRel(0, 0); }
	Vec TopRight() { return GetPosRel(1, 0); }
	Vec Left() { return GetPosRel(0, 0.5f); }
	Vec Right() { return GetPosRel(1, 0.5f); }
	Vec BottomLeft() { return GetPosRel(0, 1); }
	Vec BottomRight() { return GetPosRel(1, 1); }

	Area Move(Vec delta) {
		Area newArea = *this;
		newArea.min += delta;
		newArea.max += delta;
		return newArea;
	}

	Area MoveRel(float xRel, float yRel) {
		Vec moveVec = Vec(xRel * GetWidth(), yRel * GetHeight());
		return Move(moveVec);
	}

	Area MoveTo(Vec pos, bool center = false) {
		Area newArea = *this;

		if (center)
			pos -= GetSize() / 2;

		newArea.min = pos;
		newArea.max = pos + GetSize();

		return newArea;
	}
};