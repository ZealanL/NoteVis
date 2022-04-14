#pragma once

// Base defines - should be changed
#define PROGRAM_NAME	"NoteVis"
#define PROGRAM_VERSION "v1.0"
#define DEFAULT_WINDOWSIZE_X 720
#define DEFAULT_WINDOWSIZE_Y 480

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

// Remove need for std namespace scope for very common datatypes
using std::vector;
using std::map;
using std::unordered_map;
using std::set;
using std::unordered_set;
using std::list;
using std::stack;
using std::deque;
using std::string;
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
#define PLAT_WINDOWS
#endif

#ifdef PLAT_WINDOWS
#ifdef _WIN64
#define IS_64BIT
#endif
#else
#endif

#ifdef IS_64BIT
#define ARCH_LABEL "x64"
#else
#define ARCH_LABEL "x86"
#endif

#define EXIT(code) exit(code)

#define EXITCODE_BAD 0
#define EXITCODE_GOOD 1

#ifdef PLAT_WINDOWS
#include <windows.h>
// Show an error
#define ERROR(s) { MessageBoxA(0, ("ERROR: " + FMT(s)).c_str(), PROGRAM_NAME " Error", MB_ICONERROR); }
// Show an error and close
#define ERRORCLOSE(s)  { MessageBoxA(0, ("ERROR: " + FMT(s) + "\n" PROGRAM_NAME " will now exit.").c_str(), PROGRAM_NAME " Critical Error", MB_ICONERROR); EXIT(EXITCODE_BAD); }
#endif

#define MERGE_IDR_INNER(a, b) a##b
#define MERGE_IDR(a, b) MERGE_IDR_INNER(a, b)

// Simple structure for running a function on initialization via constructor
#define INIT_FUNC(f, nameAppend) const static volatile auto MERGE_IDR(__RunFuncOnInit_, __COUNTER__) = __RFOI(f); 
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

// For scoped enums that shouldn't be classes
#define ENUM_SCOPE(name, ...) namespace name { enum {##__VA_ARGS__}; }