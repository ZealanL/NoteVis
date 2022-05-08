#include "Framework.h"

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

string FW::Narrow(wstring wstr) {
	return string(wstr.begin(), wstr.end());
}

void FW::ShowError(string title, string text) {
#ifdef PLAT_WINDOWS
	MessageBoxA(0, ("Error: " + text).c_str(), title.c_str(), MB_ICONERROR);
#else
	// TODO: Implement
#endif
}

bool FW::ShowWarning(string title, string text, bool yesNo) {
#ifdef PLAT_WINDOWS
	int result = MessageBoxA(0, text.c_str(), title.c_str(), yesNo ? (MB_ICONWARNING | MB_YESNO) : MB_ICONWARNING);
	return yesNo ? (result == IDYES) : true;
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

size_t FW::HashStr(string str) {
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