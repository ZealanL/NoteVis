#include "NVFileSystem.h"
#include "../Core.h"
#include <SDL_syswm.h>

// Written at the start of NoteVis score files
typedef uint32 NvMagicInt;
constexpr NvMagicInt NV_SCORE_MAGIC = 'NVS\0';

namespace fs = std::filesystem;

#ifdef PLAT_WINDOWS
#define SCORE_WINDOWS_FILE_FILTER (L"NoteVis Score Files (*" SCORE_FILE_EXTENSION ")\0*" SCORE_FILE_EXTENSION "\0")
#endif

fs::path NVFileSystem::GetDataPath() {

	fs::path currentPath = fs::current_path() / PROGRAM_NAME;

#ifdef PLAT_WINDOWS
	wchar_t* userDocumentsPath;
	if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userDocumentsPath) == S_OK) {
		currentPath = fs::path(userDocumentsPath) / PROGRAM_NAME;
	}
#endif

	return currentPath;
}

fs::path NVFileSystem::GetScoresPath() {
	return GetDataPath() / "Scores";
}


bool NVFileSystem::LoadFile(wstring fileName, ByteDataStream& dataOut) {
	std::ifstream fileIn = std::ifstream(fileName, std::ios::binary);
	if (!fileIn.good())
		return false;

	dataOut.clear();
	
	dataOut.insert(dataOut.end(), std::istreambuf_iterator<char>(fileIn), std::istreambuf_iterator<char>());
	return true;
}

bool NVFileSystem::SaveFile(wstring filePath, const ByteDataStream& dataIn) {
	fs::path fullPath = filePath;
	fs::path directoryPath = filePath;
	directoryPath.remove_filename();

	if (!fs::exists(directoryPath) && !fs::create_directories(directoryPath))
		return false;

	std::ofstream fileOut = std::ofstream(fullPath, std::ios::binary);
	if (!fileOut.good())
		return false;

	if (!dataIn.empty())
		fileOut.write((char*)dataIn.GetBasePointer(), dataIn.size());

	fileOut.close();
	return true;
}

bool NVFileSystem::SaveDataFile(wstring fileName, const ByteDataStream& dataIn) {
	return SaveDataFile(fs::relative(fileName, GetDataPath()), dataIn);
}

bool NVFileSystem::LoadDataFile(wstring fileName, ByteDataStream& dataOut) {
	return LoadFile(fs::relative(fileName, GetDataPath()), dataOut);
}

void SerializeScore(ByteDataStream& out) {
	out.Write(NV_SCORE_MAGIC);
	out.Write<uint32>(PROGRAM_VERSION_INT);

	ByteDataStream noteGraphData;
	g_NoteGraph.Serialize(noteGraphData);

	// Write notegraph data size
	out.Write<uint64>(noteGraphData.size());

	// Add all notegraph data
	out.reserve(noteGraphData.size());
	out.insert(out.end(), noteGraphData.begin(), noteGraphData.end());
}

bool DeserializeScore(ByteDataStream::ReadIterator itr) {

	if (itr.ReadVal<NvMagicInt>(-1) != NV_SCORE_MAGIC)
		return false; // Missing/invalid magic

	// Read version number
	uint32 verNum = itr.ReadVal<uint32>(-1);
	if (verNum == -1)
		return false;

	auto noteGraphDataLength = itr.ReadVal<uint64>(-1);
	if (noteGraphDataLength == -1 || noteGraphDataLength > itr.BytesLeft())
		return false;

	ByteDataStream noteGraphData = itr.ReadBytesToStream(noteGraphDataLength);
	g_NoteGraph.Deserialize(noteGraphData.GetIterator());

	return true;
}

bool NVFileSystem::OpenScore() {
#ifdef PLAT_WINDOWS
	
#else
	// TODO: Implement 
#endif
	return true;
}

// TODO: Move?
fs::path currentScoreSavePath = fs::path();

bool NVFileSystem::SaveScore() {
	if (currentScoreSavePath.empty()) {
		SaveScoreAs();
	} else {
		ByteDataStream scoreData;
		SerializeScore(scoreData);
		if (!SaveFile(currentScoreSavePath, scoreData)) {
			// TODO: Show error
		}
	}

	return true;
}

bool NVFileSystem::SaveScoreAs() {
	fs::path savePath;

	auto scoresPath = GetScoresPath();
	if (!fs::exists(scoresPath)) {
		try {
			fs::create_directories(scoresPath);
		} catch (std::exception& e) {
			return false;
		}
	}

#ifdef PLAT_WINDOWS
	{
		wchar_t savePathBuffer[MAX_PATH]{};
		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(ofn);

		// Set window handle to our main SDL window
		SDL_SysWMinfo sysInfo;
		SDL_VERSION(&sysInfo.version);
		SDL_GetWindowWMInfo(g_SDL_Window, &sysInfo);
		ofn.hwndOwner = sysInfo.info.win.window;
	
		wstring scoresPathStr = GetScoresPath();
		ofn.lpstrInitialDir = scoresPathStr.c_str();

		ofn.lpstrFilter = SCORE_WINDOWS_FILE_FILTER;
		ofn.lpstrFile = savePathBuffer;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = L"Save Score As...";
		ofn.Flags = OFN_DONTADDTORECENT;

		if (!GetSaveFileNameW(&ofn))
			return false;

		savePath = savePathBuffer;
		savePath += SCORE_FILE_EXTENSION;
	}
#else
	// TODO: Implement 
#endif

	ByteDataStream scoreData; SerializeScore(scoreData);
	if (SaveFile(savePath, scoreData)) {
		currentScoreSavePath = savePath;
		return true;
	} else {
		return false;
	}
}