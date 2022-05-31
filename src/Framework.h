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
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// fmt::format (in place of unstable std::format, which not all compilers support yet)
#include <fmt/format.h>
#include <fmt/xchar.h>
#define FMT fmt::format

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"

// Integer typedefs
typedef int8_t	int8;	typedef uint8_t	 uint8;
typedef int16_t int16;	typedef uint16_t uint16;
typedef int32_t int32;	typedef uint32_t uint32;
typedef int64_t int64;	typedef uint64_t uint64;
typedef uint8_t byte;

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__CYGWIN__)
#define PLAT_WINDOWS
#endif

#if defined(_WIN64) || defined(__x86_64__)
#define PLAT_64
#endif

#ifdef PLAT_64
#define ARCH_LABEL "x64"
#else
#define ARCH_LABEL "x86"
#endif

#if _MSC_VER and !defined(__llvm__) and !defined(__clang__)
#define PLAT_MSVC
#endif

#define EXITCODE_BAD 0
#define EXITCODE_GOOD 1
#define EXIT(code) exit(code)

// Debug logging
#ifdef _DEBUG
#define DLOG FW::PrintConsoleLine
#else
#define DLOG(s, ...) {}
#endif

#ifdef PLAT_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>

// Defines we don't want
#undef IGNORE
#undef ERROR
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) (sizeof(arr)/sizeof(*arr))
#endif

#define MERGE_IDR_INNER(a, b) a##b
#define MERGE_IDR(a, b) MERGE_IDR_INNER(a, b)

// Simple structure for running a function on initialization via constructor
#define INIT_FUNC(f) const static volatile auto MERGE_IDR(__RunFuncOnInit_, __COUNTER__) = __RFOI(f); 
struct __RFOI { __RFOI(std::function<void()> fnRunFunc) { fnRunFunc(); } };

// Debug assertion
#define SASSERT static_assert
#define ASSERT assert
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
#ifdef PLAT_MSVC
#define ENUM_SCOPE(name, ...) namespace name { enum {##__VA_ARGS__}; }
#else
#define ENUM_SCOPE(name, ...) namespace name { enum {__VA_ARGS__}; }
#endif

#define CURRENT_TIME (FW::GetCurTime())

// Member property that is publicly constant but only modifiable privately
#define PRIVCONST(type, name, val) \
private: type _##name = val; \
public: const type& name = _##name

#define PRIVCONST_ARR(type, name, size, val) \
private: type _##name[size] = val; \
public: const type* name = _##name

#define CWIDEN(str) (L"" str)

// Framework functions
namespace FW {
	string EncodeUTF8(wstring wstr);
	wstring DecodeUTF8(string wstr);
	wstring Widen(string str);
	string Flatten(wstring wstr);

	enum class MsgBoxType : uint32 {
		INFO = SDL_MESSAGEBOX_INFORMATION,
		WARNING = SDL_MESSAGEBOX_WARNING,
		ERROR = SDL_MESSAGEBOX_ERROR,
	};

	enum MsgBoxButton : uint32 {
		MBB_INVALID,
		MBB_OK,
		MBB_CANCEL,
		MBB_YES,
		MBB_NO,
		MBB_ABORT,
		MBB_IGNORE,
		MBB_RETRY,

		MBB_AMOUNT,
	};

	enum MsgBoxButtons : uint32 {
		MBBS_OK = (1 << MBB_OK),
		MBBS_OK_CANCEL = (1 << MBB_OK) | (1 << MBB_CANCEL),
		MBBS_YES_NO = (1 << MBB_YES) | (1 << MBB_NO),
		MBBS_YES_NO_CANCEL = (1 << MBB_YES) | (1 << MBB_NO) | (1 << MBB_CANCEL),
	};

	MsgBoxButton ShowMsgBox(wstring title, wstring text, MsgBoxType type = MsgBoxType::INFO, MsgBoxButtons buttons = MBBS_OK);
	MsgBoxButton ShowMsgBox(string title, string text, MsgBoxType type = MsgBoxType::INFO, MsgBoxButtons buttons = MBBS_OK);
	void ShowError(string title, string text);
	void ShowError(string title, wstring text);

	bool WarnYesNo(string title, string text);

	template<class ...Args>
	void FatalError(Args... args) {
		FW::ShowError("Fatal Error", FMT(args...) + "\n\n" PROGRAM_NAME " will now exit.");
		EXIT(EXITCODE_BAD);
	}

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

	template<typename... Args>
	void PrintConsoleLine(Args&&... args) {
		auto formatResult = FMT(args...);
		std::wcout << wstring(formatResult.begin(), formatResult.end()) << std::endl;
	}

	// Compile-time string hashing
#define COMPSTRHASH(s) FW::HashDataConstExpr<sizeof(s)>(s)
}