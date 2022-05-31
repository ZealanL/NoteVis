#include "LogNotif.h"

#include "../../../Render/Draw/Draw.h"

void LogNotifList::Add(string text, double lifeDuration) {
	_notifs.push_back(LogNotif{ text, lifeDuration, lifeDuration / 4, CURRENT_TIME });
	if (_notifs.size() > maxNotifs)
		_notifs.pop_front();
}

void LogNotifList::DrawAndUpdate(Vec topLeftStart, float maxWidth) {

	static Color backColor = Color(10, 10, 10);
	constexpr float padding = 8;

	float textAreaWidth = maxWidth - padding * 2;
	if (textAreaWidth <= 0) {
		ASSERT(false); // Not enough space to draw anything
		return;
	}

	// Remove expired prior to drawing
	while (!_notifs.empty() && _notifs.front().IsExpired())
		_notifs.pop_front();

	float currentY = topLeftStart.y;
	for (auto& notif : _notifs) {
		ASSERT(!notif.IsExpired());

		float height = Draw::GetTextSize(notif.text, textAreaWidth).y;
		Area backArea = Area(Vec(topLeftStart.x, currentY), Vec(topLeftStart.x + maxWidth, currentY + height + padding * 2));
		Area textArea = backArea.Expand(-padding);

		Draw::Rect(backArea, Color(10, 10, 10, notif.GetAlpha() / 2));
		Draw::ORect(backArea, Color(255, 255, 255, notif.GetAlpha() / 2));
		Draw::TextInArea(notif.text, textArea, Color(255, 255, 255, notif.GetAlpha()));

		currentY += MAX(backArea.Height() - 1, 0) * MIN((notif.GetAlpha() / 100.f), 1);
	}
}