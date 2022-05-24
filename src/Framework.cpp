#include "Framework.h"
#include <SDL_syswm.h>

auto startTime = std::chrono::system_clock::now();

double FW::GetCurTime() {
	using namespace std::chrono;
	auto timeSinceStartup = system_clock::now() - startTime;

	double microsecs = duration_cast<microseconds>(timeSinceStartup).count();
    return microsecs / (1000.0 * 1000.0);
}

wstring FW::Widen(string str) {
	return wstring(str.begin(), str.end());
}

string FW::Flatten(wstring wstr) {
	return string(wstr.begin(), wstr.end());
}

void FW::ShowError(string title, string text) {
	ShowError(title, Widen(text));
}

void FW::ShowError(string title, wstring text) {
	ShowMsgBox(Widen(title), text);
}

bool FW::WarnYesNo(string title, string text) {
	return FW::ShowMsgBox(title, text, MsgBoxType::WARNING, MsgBoxButtons::YES_NO) == MsgBoxResult::YES;
}

FW::MsgBoxResult FW::ShowMsgBox(string title, string text, MsgBoxType type, MsgBoxButtons buttons) {
	return FW::ShowMsgBox(Widen(title), Widen(text), type, buttons);
}

FW::MsgBoxResult FW::ShowMsgBox(wstring title, wstring text, MsgBoxType type, MsgBoxButtons buttons) {
	using namespace FW;

	// Not using SDL messagebox because SDL initialization is not guarenteed

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	bool getWindowInfoSucceeded = SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &wmInfo);

#ifdef PLAT_WINDOWS

	MsgBoxResult result = (MsgBoxResult)MessageBoxW(
		getWindowInfoSucceeded ? wmInfo.info.win.window : NULL,
		text.c_str(), title.c_str(),
		(uint32)type | (uint32)buttons);

	return result;
#else
	// TODO: Implement
#endif
}

string FW::TimeDurationToString(double time) {
	double mins = time / 60;
	double hours = mins / 60;
	double days = hours / 24;
	double years = days / 365;

	if (years > 1)
		return FMT("{:.2f} years", years);

	if (days > 1)
		return FMT("{:.2f} days", days);

	if (hours > 1)
		return FMT("{:.2f} hours", hours);

	if (mins > 1)
		return FMT("{:.2f} mins", mins);

	if (time > 0.1)
		return FMT("{:.2f} secs", time);
	
	return FMT("{:.2f} ms", time*1000.0);
}

FW::HASH FW::HashData(const void* ptr, int size) {
	HASH result = 0;
	constexpr HASH prime = 31;
	for (HASH i = 0; i < size; ++i) {
		result = ((BYTE*)ptr)[i] + (result * prime);
	}
	return result;
}

FW::HASH FW::HashStr(string str) {
	return HashData(str.c_str(), str.size());
}

string FW::NumOrdinal(uint64 val) {
	string out = std::to_string(val);

	uint64 base100 = val % 100;
	if (base100 <= 10 || base100 >= 20) {
		switch (val % 10) {
		case 1:
			return out + "st";
		case 2:
			return out + "nd";
		case 3:
			return out + "rd";
		}
	}
	
	return out + "th";
}