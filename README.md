# NoteVis
A colorful music composition editor and sightreading program written in C++.

Libraries used:
- https://www.libsdl.org/ for backend rendering (OpenGL)
- https://github.com/ocornut/imgui for higher level rendering and UI (may be replaced in the future)
- https://github.com/jcelerier/libremidi for MIDI file IO
- https://zlib.net/ for convenient compression/decompression
- https://github.com/fmtlib/fmt for easy string formatting, in place of the sometimes-unsupported `std::format`