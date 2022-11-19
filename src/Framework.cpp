#include "Framework.h"

#include <locale>
#include <codecvt>

auto startTime = std::chrono::system_clock::now();

double FW::GetCurTime() {
	using namespace std::chrono;
	auto timeSinceStartup = system_clock::now() - startTime;

	double microsecs = duration_cast<microseconds>(timeSinceStartup).count();
    return microsecs / (1000.0 * 1000.0);
}

string FW::EncodeUTF8(wstring wstr) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

wstring FW::DecodeUTF8(string wstr) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(wstr);
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
	return FW::ShowMsgBox(title, text, MsgBoxType::WARNING, MBBS_YES_NO) == MBB_YES;
}

FW::MsgBoxButton FW::ShowMsgBox(string title, string text, MsgBoxType type, MsgBoxButtons buttons) {
	return FW::ShowMsgBox(Widen(title), Widen(text), type, buttons);
}

FW::MsgBoxButton FW::ShowMsgBox(wstring title, wstring text, MsgBoxType type, MsgBoxButtons buttons) {
	using namespace FW;

	string u8title = EncodeUTF8(title), u8text = EncodeUTF8(text);

	constexpr const char* buttonNames[] = {
		"INVALID", "Yes", "No", "Abort", "Retry", "Ignore", "Ok", "Cancel",
	};

	SASSERT(ARRAYSIZE(buttonNames) == MBB_AMOUNT, "buttonNames[] needs to be updated to match MsgBoxButtons enum");

	vector<SDL_MessageBoxButtonData> sdlButtons;
	for (int i = 0; i < MBB_AMOUNT; i++) {
		if (buttons & (1<<i)) {
			sdlButtons.push_back(
				SDL_MessageBoxButtonData{
					0,
					i,
					buttonNames[i]
				}
			);
		}
	}

	// Make buttons appear in the right order
	std::reverse(sdlButtons.begin(), sdlButtons.end());

	ASSERT(!sdlButtons.empty());

	// Casts are needed here to ensure compilation on different compilers
	SDL_MessageBoxData messageBoxData = {
		(uint32)type,
		SDL_GL_GetCurrentWindow(),
		(const char*)u8title.c_str(),
		(const char*)u8text.c_str(),
		(int32)sdlButtons.size(),
		&sdlButtons.front(),
		NULL
	};

	int buttonOut;
	SDL_ShowMessageBox(&messageBoxData, &buttonOut);
	return (MsgBoxButton)buttonOut;
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
		result = ((byte*)ptr)[i] + (result * prime);
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