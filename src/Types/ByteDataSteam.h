#pragma once
#include "../Framework.h"

#include <streambuf>
#include <istream>

// Adapted from https://stackoverflow.com/questions/13059091/creating-an-input-stream-from-constant-memory/13059195#13059195
struct ByteStreamBuf : std::streambuf {
	ByteStreamBuf(const void* base, size_t size) {
		char* p = (char*)base;
		this->setg(p, p, p + size);
	}
};

// Adapted from https://stackoverflow.com/questions/13059091/creating-an-input-stream-from-constant-memory/13059195#13059195
struct IByteStream : virtual ByteStreamBuf, std::istream {
	IByteStream(char const* base, size_t size)
		: ByteStreamBuf(base, size)
		, std::istream((std::streambuf*)(this)) {
	}
};

// Steam of bytes for serialization
class ByteDataStream : public vector<byte> {
public:

	struct ReadIterator : virtual ByteStreamBuf, std::istream {
		const ByteDataStream* stream;
		uint64 curIndex;

		ReadIterator(const ByteDataStream* stream) : 
			stream(stream), 
			ByteStreamBuf(stream->GetBasePointer(), stream->size()),
			std::istream((std::streambuf*)(this)) 
		{
			curIndex = 0;
		}

		int BytesLeft() {
			return MAX(0, stream->size() - curIndex);
		}

		// Returns number of bytes read
		uint64 ReadBytesToStream(ByteDataStream& streamOut, uint64 maxReadSize);

		template<typename T>
		bool Read(T* out) {
			bool result = stream->ReadFromBytes(curIndex, out);
			if (result)
				curIndex += sizeof(T);
			else 
				curIndex = stream->size();
			return result;
		}

		template<typename T>
		T ReadVal(T defaultVal) {
			T valResult{};
			bool result = Read(&valResult);
			return result ? valResult : defaultVal;
		}

		// Read, then reverse the byte order
		template<typename T>
		T ReadValInv(T defaultVal) {
			T result = ReadVal<T>(defaultVal);
			byte* start = (byte*)&result;
			byte* end = start + sizeof(T);
			std::reverse(start, end);
			return result;
		}

		string ReadString();
	};

	byte* GetBasePointer() const {
		return this->empty() ? (NULL) : (byte*)&(this->front());
	}

	template <typename T>
	void Write(T object) {
		this->insert(this->end(), (byte*)&object, ((byte*)&object) + sizeof(T));
	}

	// Write with inversed endian-ness
	template <typename T>
	void WriteInv(T object) {
		for (int i = sizeof(T) - 1; i >= 0; i--) {
			this->push_back(((byte*)&object)[i]);
		}
	}

	void WriteString(string str) {
		this->insert(this->end(), str.begin(), str.end());
		this->push_back(NULL); // Add terminator
	}

	// Returns true if bytes were read, false if out of range (is negative-index proof)
	template <typename T>
	bool ReadFromBytes(int index, T* out) const {
		if (index < 0 || index + sizeof(T) >= this->size())
			return false;

		memcpy(out, GetBasePointer() + index, sizeof(T));
		return true;
	}

	ReadIterator GetIterator() const {
		return ReadIterator(this);
	}

	IByteStream MakeIStream() {
		return IByteStream(GetBasePointer(), size());
	}

	void ReadFromFileStream(std::ifstream& streamIn);

	void WriteToFileStream(std::ofstream& streamOut) const;

	bool DataMatches(const ByteDataStream& other) const;

	FW::HASH CalculateHash() {
		return this->empty() ? NULL : FW::HashData(this->GetBasePointer(), this->size());
	}

	bool Compress();
	bool Decompress();
};

// Stores the difference from one ByteDataSteam to the next 
// TODO: Implement
class ByteDataStreamDiff {
public:
	vector<pair<size_t, byte>> additions;
	vector<size_t> removals;
	vector<pair<size_t, byte>> changes;
};