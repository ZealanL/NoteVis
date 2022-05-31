#include "ByteDataSteam.h"
#include "../ZLibWrapper/ZLibWrapper.h"

void ByteDataStream::ReadFromFileStream(std::ifstream& streamIn) {
	streamIn.seekg(0, streamIn.end);
	size_t fileSize = streamIn.tellg();
	streamIn.seekg(0, streamIn.beg);

	if (fileSize > 0) {
		this->resize(fileSize);
		streamIn.read((char*)GetBasePointer(), fileSize);
	}
}

void ByteDataStream::WriteToFileStream(std::ofstream& streamOut) const {
	if (!this->empty())
		streamOut.write((char*)GetBasePointer(), this->size());
}

bool ByteDataStream::Compress() {
	uint64 maxCompressedSize = ZLib::GetMaxCompressedSize(this->size());
	byte* compressedData = (byte*)malloc(maxCompressedSize);
	uint64 compressedSize = ZLib::Compress(this->GetBasePointer(), this->size(), compressedData);

	if (compressedSize) {
		uint64 decompressedSize = this->size();
		this->resize(compressedSize + sizeof(uint64));
		memcpy(this->GetBasePointer(), &decompressedSize, sizeof(uint64));
		memcpy(this->GetBasePointer() + sizeof(uint64), compressedData, compressedSize);
		free(compressedData);
		return true;
	} else {
		free(compressedData);
		return false;
	}
}

bool ByteDataStream::Decompress() {
	uint64 decompressedSize;
	if (!this->ReadFromBytes<uint64>(0, &decompressedSize))
		return false; // Not enough space

	if (decompressedSize == 0) {
		this->clear();
		return true;
	}

	byte* decompressedData = (byte*)malloc(decompressedSize);

	uint64 actualDecompressedSize = ZLib::Decompress(this->GetBasePointer() + sizeof(uint64), this->size() - sizeof(uint64), decompressedData);

	if (actualDecompressedSize != decompressedSize) {
		free(decompressedData);
		return false;
	}

	this->resize(decompressedSize);
	memcpy(this->GetBasePointer(), decompressedData, actualDecompressedSize);
	free(decompressedData);
	return true;
}

bool ByteDataStream::DataMatches(const ByteDataStream& other) const {
	if (this->empty() || other.empty()) {
		return this->empty() == other.empty();
	} else {
		return (this->size() == other.size()) && !memcmp(this->GetBasePointer(), other.GetBasePointer(), this->size());
	}
}

string ByteDataStream::ReadIterator::ReadString() {
	std::stringstream strData;
	char charOut;
	while (Read(&charOut) && charOut) {
		strData << charOut;
	}
	return strData.str();
}

uint64 ByteDataStream::ReadIterator::ReadBytesToStream(ByteDataStream& streamOut, uint64 maxReadSize) {
	uint64 readSize = MIN(BytesLeft(), maxReadSize);

	if (readSize > 0)
	streamOut.insert(streamOut.end(), stream->begin() + curIndex, stream->begin() + curIndex + readSize);

	curIndex += readSize;
	return readSize;
}