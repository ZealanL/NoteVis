// ZLibWrapper exists to keep ZLib functions isolated (since they have no namespace)
// TODO: Update ZLib, this version has an exploit?

#pragma once
#include "../Framework.h"

namespace ZLib {
	uint64 Compress(void* data, uint64 dataSize, void* compressedOutput);
	uint64 Decompress(void* data, uint64 dataSize, void* decompressedOutput);

	uint64 GetMaxCompressedSize(uint64 dataSize);
}