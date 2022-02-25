#pragma once

#define NV_VERSION "1.0"

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
#include <list>

// Remove need for std prefixf for very common types
using std::vector;
using std::map;
using std::set;
using std::list;
using std::string;
#pragma endregion

// SDL 2
#include <SDL.h>
#include <SDL_opengl.h>

// IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"

#define FINLINE __forceinline

// Debug logging
#ifdef _DEBUG
#define DLOG(s) { std::cout << "> " << std::dec << s << std::endl; }
#else
#define DLOG(s) {}
#endif

// Create a string from elements concat
#define STR(s) ([=]{std::stringstream _strmacro_stream; _strmacro_stream << s; return _strmacro_stream.str();}())

typedef unsigned char BYTE;

#define EXITCODE_FAIL 0
#define EXITCODE_GOOD 1

// Debug assertion
#define NV_SASSERT(e) static_assert(e)
#define NV_ASSERT(e) assert(e)
#define NV_IASSERT(e, s) NV_ASSERT(e >= 0 && e < s) // Index assert 

#ifdef NV_WINDOWS
// Show an error
#define ERROR(s) {MessageBoxA(0, STR("ERROR: " << s).c_str(), "NoteVis " NV_VERSION " Error", MB_ICONERROR);}
// Show an error and close
#define ERRORCLOSE(s)  { MessageBoxA(0, STR("ERROR: " << s << std::endl << "NoteVis will now exit.").c_str(), "NoteVis " NV_VERSION " Critical Error", MB_ICONERROR); exit(0); }
#endif

// Quick max/min logic
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#define MIN3(a, b, c) MIN(MIN(a, b), c)

// Simple structure for running a function on initialization via constructor
#define INIT_FUNC(f, nameAppend) const volatile auto _RunFuncOninit##nameAppend = RunFuncOnInit(f); 
struct RunFuncOnInit {
	RunFuncOnInit(std::function<void()> runFunc) {
		runFunc();
	}
	
};