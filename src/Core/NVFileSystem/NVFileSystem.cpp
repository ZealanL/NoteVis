#include "NVFileSystem.h"
#include "../Core.h"
#include <SDL_syswm.h>

// Written at the start of NoteVis score files
typedef uint32 NvMagicInt;
constexpr NvMagicInt NV_SCORE_MAGIC = '\0SVN'; // "NVS";

namespace fs = std::filesystem;

#ifdef PLAT_WINDOWS
#define SCORE_WINDOWS_FILE_FILTER (L"NoteVis Score Files (*" SCORE_FILE_EXTENSION ")\0*" SCORE_FILE_EXTENSION "\0")
#endif

fs::path NVFileSystem::FilePrompt(string promptTitle, fs::path initialPath, string fileTypeLabel, string fileTypeExtension, bool save) {
	fs::path resultPath;

#ifdef PLAT_WINDOWS
	{
		wchar_t pathBuffer[MAX_PATH]{};
		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(ofn);

		// Set window handle to our main SDL window
		SDL_SysWMinfo sysInfo;
		SDL_VERSION(&sysInfo.version);
		SDL_GetWindowWMInfo(g_SDL_Window, &sysInfo);
		ofn.hwndOwner = sysInfo.info.win.window;

		if (!initialPath.empty()) {
			ofn.lpstrInitialDir = initialPath.c_str();
		}

		wstring filterStr = FW::Widen(fileTypeLabel + " (*" + fileTypeExtension + ")") 
			+ L'\0' + L'*' + FW::Widen(fileTypeExtension) + L'\0';

		ofn.lpstrFilter = filterStr.c_str();
		ofn.lpstrFile = pathBuffer;
		ofn.nMaxFile = MAX_PATH;

		wstring wPromptTitle = FW::Widen(promptTitle);
		ofn.lpstrTitle = wPromptTitle.c_str();
		ofn.Flags = OFN_DONTADDTORECENT | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);

		if (!(save ? GetSaveFileNameW : GetOpenFileNameW)(&ofn))
			return fs::path();

		resultPath = pathBuffer;
		if (save)
			resultPath.replace_extension(wstring(fileTypeExtension.begin(), fileTypeExtension.end()));
	}
#else
	// TODO: Implement 
#endif

	return resultPath;
}

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

	try {
		if (!fs::exists(directoryPath) && !fs::create_directories(directoryPath))
			return false;
	} catch (std::exception& e) {
		return false;
	}

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
	out.Write<uint64>((uint64)noteGraphData.size());

	// Add all notegraph data
	out.reserve(noteGraphData.size());
	out.insert(out.end(), noteGraphData.begin(), noteGraphData.end());

	bool compressResult = out.Compress();
	ASSERT(compressResult);
}

bool DeserializeScore(ByteDataStream& data) {
	if (!data.Decompress())
		return false;

	auto itr = data.GetIterator();

	if (itr.ReadVal<NvMagicInt>(-1) != NV_SCORE_MAGIC)
		return false; // Missing/invalid magic

	// Read version number
	uint32 verNum = itr.ReadVal<uint32>(-1);
	if (verNum == -1)
		return false;

	auto noteGraphDataLength = itr.ReadVal<uint64>(-1);
	if (noteGraphDataLength == -1 || noteGraphDataLength > itr.BytesLeft())
		return false;

	ByteDataStream noteGraphData;
	if (itr.ReadBytesToStream(noteGraphData, noteGraphDataLength) != noteGraphDataLength)
		return false;
	g_NoteGraph.Deserialize(noteGraphData.GetIterator());

	return true;
}

bool NVFileSystem::TryCloseScore(bool force) {
	if (!force && g_HasUnsavedChanges) {
		if (!FW::WarnYesNo("Unsaved Changes", "This score has unsaved changes.\nClose it anyway?"))
			return false;
	}

	g_NoteGraph.ClearEverything(false);

	return true;
}

bool NVFileSystem::OpenScore() {
	constexpr auto ERROR_TITLE = "Score Loading Error";

	fs::path openPath = FilePrompt("Open Score...", GetScoresPath(), "NoteVis Scores", SCORE_FILE_EXTENSION, false);

	ByteDataStream scoreData;
	if (!LoadFile(openPath, scoreData)) {
		FW::ShowError(ERROR_TITLE, std::format(L"Failed to load score file from \"{}\".", openPath.wstring()));
		return false;
	}

	if (!TryCloseScore())
		return false;

	bool result = DeserializeScore(scoreData);
	if (result) {
		g_ScoreSavePath = openPath;
		g_HasUnsavedChanges = false;
		NG_NOTIF("Loaded \"{}\"", GetCurrentScoreName());
		return true;
	} else {
		FW::ShowError(ERROR_TITLE, std::format(L"Failed read score data from \"{}\".\nFile is likely corrupt.", openPath.wstring()));
		return false;
	}
}

string NVFileSystem::GetCurrentScoreName() {
	fs::path scoreName = g_ScoreSavePath.filename();
	if (scoreName.empty()) {
		return "Untitled Score";
	} else {
		return scoreName.string();
	}
}

bool NVFileSystem::SaveScore() {
	constexpr auto ERROR_TITLE = "Score Saving Error";

	if (g_ScoreSavePath.empty()) {
		SaveScoreAs();
	} else {
		ByteDataStream scoreData;
		SerializeScore(scoreData);
		if (!SaveFile(g_ScoreSavePath, scoreData)) {
			FW::ShowError(ERROR_TITLE, std::format(L"Failed to save score to \"{}\".\nIs it open elsewhere?", g_ScoreSavePath.wstring()));
			return false;
		} else {
			NG_NOTIF("Saved \"{}\"", GetCurrentScoreName());
			g_HasUnsavedChanges = false;
		}
	}

	return true;
}

bool NVFileSystem::SaveScoreAs() {
	constexpr auto ERROR_TITLE = "Score Saving Error";

	auto scoresPath = GetScoresPath();
	if (!fs::exists(scoresPath)) {
		try {
			fs::create_directories(scoresPath);
		} catch (std::exception& e) {
			FW::ShowError(ERROR_TITLE, std::format(L"Failed to create folder for scores at \"{}\".", scoresPath.wstring()));
			return false;
		}
	}

	fs::path savePath = FilePrompt("Save Score...", GetScoresPath(), "NoteVis Scores", SCORE_FILE_EXTENSION, true);
	if (savePath.empty())
		return false;

	g_ScoreSavePath = savePath;
	return SaveScore();
}