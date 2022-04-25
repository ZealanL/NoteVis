#pragma once
#include "../../../Framework.h"
#include "../../../Types/Vec.h"

// A small dropdown notification/log message
struct LogNotif {
	string text;
	double lifeDuration; // How long to show for
	double fadeDuration; // How long to spend fading out
	double startTime;

	double GetTimeAlive() {
		return CURRENT_TIME - startTime;
	}

	bool IsExpired() {
		return startTime > CURRENT_TIME || GetTimeAlive() > lifeDuration;
	}

	BYTE GetAlpha() {
		if (IsExpired()) {
			return 255;
		} else {
			float fadeRatio = CLAMP((lifeDuration - GetTimeAlive()) - fadeDuration, 0, 1);
			return fadeRatio * 255;
		}
	}
};

struct LogNotifList {
	deque<LogNotif> _notifs;
	int maxNotifs = 10;

	void Add(string text, double lifeDuration = 5.0);
	void DrawAndUpdate(Vec topLeftStart, float maxWidth);
};