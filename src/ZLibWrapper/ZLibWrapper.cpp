#include "ZLibWrapper.h"

#include <zlib/zlib.h>

uint64 ZLib::Compress(void* data, uint64 dataSize, void* compressedOutput) {
	uLongf sizeOut;
	try {
		compress((Bytef*)compressedOutput, &sizeOut, (Bytef*)data, dataSize);
		return sizeOut;
	} catch (std::exception& e) {
		return NULL;
	}
}

uint64 ZLib::Decompress(void* data, uint64 dataSize, void* decompressedOutput) {
	uLongf sizeOut;
	try {
		int result = uncompress((Bytef*)decompressedOutput, &sizeOut, (Bytef*)data, dataSize);
		return sizeOut;
	} catch (std::exception& e) {
		return NULL;
	}
}

uint64 ZLib::GetMaxCompressedSize(uint64 dataSize) {
	return compressBound(dataSize);
}
