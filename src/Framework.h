#pragma once

// Base defines - should be changed accordingly
#define PROGRAM_NAME	"NoteVis"
#define PROGRAM_VERSION "v1.0"

#define PROGRAM_VERSION_MAJOR 1
#define PROGRAM_VERSION_MINOR 0
#define PROGRAM_VERSION_INT ((uint32)(PROGRAM_VERSION_MAJOR * 10000 + PROGRAM_VERSION_MINOR))

#define DEFAULT_WINDOWSIZE_X 1280
#define DEFAULT_WINDOWSIZE_Y 720

#pragma region STD Includes
#define _HAS_STD_BYTE false
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <string>
#include <functional>
#include <format>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <iomanip>
#include <chrono>
#include <filesystem>

// Remove need for std namespace scope for very common datatypes
using std::vector;
using std::map;
using std::unordered_map;
using std::set;
using std::multiset;
using std::unordered_set;
using std::list;
using std::stack;
using std::deque;
using std::string;
using std::wstring;
using std::pair;
#pragma endregion

// SDL 2 + OpenGL
#include <SDL.h>
#include <SDL_opengl.h>

// ImGui
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_sdl.h"

// libremidi
#include "../libremidi/libremidi.hpp"
#include "../libremidi/reader.hpp"

// Integer typedefs
typedef int8_t	int8;	typedef uint8_t	 uint8;
typedef int16_t int16;	typedef uint16_t uint16;
typedef int32_t int32;	typedef uint32_t uint32;
typedef int64_t int64;	typedef uint64_t uint64;
typedef BYTE uint8_t;

// Debug logging
#ifdef _DEBUG
#define DLOG(s, ...) { std::cout << "> " << std::dec << std::format(s, ##__VA_ARGS__) << std::endl; }
#else
#define DLOG(s) {}
#endif

// String formatting
#define FMT(s, ...) std::format(s, ##__VA_ARGS__)

#if defined(WIN32) || defined(_WIN64) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#ifndef PLAT_WINDOWS
#error PLAT_WINDOWS should be predefined for Windows builds
#endif
#else
#error PLAT_WINDOWS should only be predefined for Windows builds
#endif

#if defined(_WIN64)
#define IS_64BIT
#endif

#ifdef IS_64BIT
#define ARCH_LABEL "x64"
#else
#define ARCH_LABEL "x86"
#endif

#define EXITCODE_BAD 0
#define EXITCODE_GOOD 1
#define EXIT(code) exit(code)

#ifdef PLAT_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#endif

#define MERGE_IDR_INNER(a, b) a##b
#define MERGE_IDR(a, b) MERGE_IDR_INNER(a, b)

// Simple structure for running a function on initialization via constructor
#define INIT_FUNC(f) const static volatile auto MERGE_IDR(__RunFuncOnInit_, __COUNTER__) = __RFOI(f); 
struct __RFOI { __RFOI(std::function<void()> fnRunFunc) { fnRunFunc(); } };

// Debug assertion
#define SASSERT(e) static_assert(e)
#define ASSERT(e) assert(e)
#define IASSERT(e, s) ASSERT(e >= 0 && e < s) // Index assert 

// Quick max/min/clamp logic macros
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#define MIN3(a, b, c) MIN(MIN(a, b), c)
#define CLAMP(v, min, max) MAX(min, MIN(v, max))

// Snap a number to a rounding interval
#define ISNAP(val, roundingInterval) FW::IntSnap(val, roundingInterval)
#define FSNAP(val, roundingInterval) fmodf(val, roundingInterval)

// For scoped enums that shouldn't be classes
#define ENUM_SCOPE(name, ...) namespace name { enum {##__VA_ARGS__}; }

#define CURRENT_TIME (FW::GetCurTime())

// Member property that is publicly constant but only modifiable privately
#define PRIVCONST(type, name, val) \
private: type _##name = val; \
public: const type& name = _##name

#define PRIVCONST_ARR(type, name, size, val) \
private: type _##name[size] = val; \
public: const type* name = _##name

#define CWIDEN(str) (L"" str)

// Show an error
#define ERROR(s, ...) FW::ShowError(PROGRAM_NAME " Error", FMT(s, ##__VA_ARGS__))

// Show an error and close
#define ERRORCLOSE(s, ...) FW::ShowError(PROGRAM_NAME " Critical Error", FMT(s, ##__VA_ARGS__), true);

// Show a warning
#define WARN(s, ...)		FW::ShowWarning(PROGRAM_NAME " Warning", FMT(s, ##__VA_ARGS__))
// Show a warning with a yes/no choice
#define WARN_YESNO(s, ...)	FW::ShowWarning(PROGRAM_NAME " Warning", FMT(s, ##__VA_ARGS__), true)

// Framework functions
namespace FW {
	wstring Widen(string str);
	string Flatten(wstring wstr);

	void ShowError(string title, string text, bool close = false);
	void ShowError(string title, wstring text, bool close = false);

	// yesNo will display "Yes" and "No" buttons and return the user's choice
	//	otherwise the message will simply display "Ok" and return true
	bool ShowWarning(string title, string text, bool yesNo = false); 
	bool ShowWarning(string title, wstring text, bool yesNo = false); 

	double GetCurTime();

	string TimeDurationToString(double time);

	typedef uint64_t HASH;
	HASH HashData(const void* ptr, int size);
	HASH HashStr(string str);

	// Get this number as an ordinal string
	// E.x. NumOrdinal(1) -> "1st", NumOrdinal(52) -> "52nd", etc.
	string NumOrdinal(uint64 val);

	template<typename T>
	std::enable_if_t<std::is_integral_v<T>, T> IntSnap(T val, T snappingInterval) {
		T a = (val / snappingInterval) * snappingInterval;
		T b = a + snappingInterval;
		return (abs(val - a) < abs(val - b)) ? a : b;
	}

	template<int SIZE>
	constexpr HASH HashDataConstExpr(const char data[SIZE]) {
		HASH result = 0;
		const HASH prime = 31;
		for (HASH i = 0; i < SIZE; ++i) {
			result = data[i] + (result * prime);
		}
		return result;
	}

	// Compile-time string hashing
#define COMPSTRHASH(s) FW::HashDataConstExpr<sizeof(s)>(s)
}