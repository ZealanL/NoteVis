#include "Framework.h"

auto startTime = std::chrono::system_clock::now();
double FW::GetCurTime() {
	using namespace std::chrono;
	auto timeSinceStartup = system_clock::now() - startTime;

	double microsecs = duration_cast<microseconds>(timeSinceStartup).count();
    return microsecs / (1000.0 * 1000.0);
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
	size_t result = 0;
	const size_t prime = 31;
	for (size_t i = 0; i < size; ++i) {
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