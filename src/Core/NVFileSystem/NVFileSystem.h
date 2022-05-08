#pragma once
#include "../../Globals.h"

#define SCORE_FILE_EXTENSION ".nvs"

namespace NVFileSystem {
	std::filesystem::path GetDataPath();
	std::filesystem::path GetScoresPath();

	bool LoadDataFile(wstring fileName, ByteDataStream& dataOut);
	bool SaveDataFile(wstring fileName, const ByteDataStream& dataIn);

	bool LoadFile(wstring filePath, ByteDataStream& dataOut);
	bool SaveFile(wstring filePath, const ByteDataStream& dataIn);

	bool OpenScore(); // Returns true if score was loaded
	bool SaveScore(); // Returns true if score was saved
	bool SaveScoreAs(); // Returns true if score was saved
}