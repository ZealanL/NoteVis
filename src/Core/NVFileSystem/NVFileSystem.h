#pragma once
#include "../../Globals.h"

#define SCORE_FILE_EXTENSION ".nvs"

namespace NVFileSystem {
	std::filesystem::path FilePrompt(string promptTitle, std::filesystem::path initialPath, string fileTypeLabel, string fileTypeExtension, bool save);

    std::filesystem::path GetDataPath();
	std::filesystem::path GetScoresPath();
	string GetCurrentScoreName();

	bool LoadDataFile(std::filesystem::path filePath, ByteDataStream& dataOut);
	bool SaveDataFile(std::filesystem::path filePath, const ByteDataStream& dataIn);

	bool LoadFile(std::filesystem::path filePath, ByteDataStream& dataOut);
	bool SaveFile(std::filesystem::path filePath, const ByteDataStream& dataIn);

	// Returns true if user allowed score to close
	// force: Close regardless of unsaved changes (will not prompt user)
	bool TryCloseScore(bool force = false);

	bool OpenScore();

	// Returns true if score was loaded
	bool SaveScore(); // Returns true if score was saved
	bool SaveScoreAs(); // Returns true if score was saved
}