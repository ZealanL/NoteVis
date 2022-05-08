#pragma once
#include "../Framework.h"

// Steam of bytes for serialization
class ByteDataStream : public vector<BYTE> {
public:
	struct ReadIterator {
		const ByteDataStream* stream;
		uint64 curIndex;

		ReadIterator(const ByteDataStream* stream) : stream(stream) {
			curIndex = 0;
		}

		int BytesLeft() {
			return MAX(0, stream->size() - curIndex);
		}

		ByteDataStream ReadBytesToStream(uint64 maxReadSize) {
			uint64 readSize = MIN(BytesLeft(), maxReadSize);
			

			ByteDataStream streamOut;
			streamOut.insert(streamOut.end(), stream->begin() + curIndex, stream->begin() + curIndex + readSize);

			curIndex += readSize;
			return streamOut;
		}

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
			bool result = stream->ReadFromBytes(curIndex, &valResult);
			return result ? valResult : defaultVal;
		}

		string ReadString() {
			std::stringstream strData;
			char charOut;
			while (Read(&charOut) && charOut) {
				strData << charOut;
			}
			return strData.str();
		}
	};

	BYTE* GetBasePointer() const {
		return this->empty() ? NULL : this->begin()._Ptr;
	}

	template <typename T>
	void Write(T object) {
		this->insert(this->end(), (BYTE*)&object, ((BYTE*)&object) + sizeof(T));
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

	void ReadFromFileStream(std::ifstream& streamIn) {
		streamIn.seekg(0, streamIn.end);
		size_t fileSize = streamIn.tellg();
		streamIn.seekg(0, streamIn.beg);

		if (fileSize > 0) {
			this->resize(fileSize);
			streamIn.read((char*)GetBasePointer(), fileSize);
		}
	}

	void WriteToFileStream(std::ofstream& streamOut) const {
		if (!this->empty())
			streamOut.write((char*)GetBasePointer(), this->size());
	}

	bool DataMatches(const ByteDataStream& other) {
		if (this->empty() || other.empty()) {
			return this->empty() == other.empty();
		} else {
			return (this->size() == other.size()) && !memcmp(this->GetBasePointer(), other.GetBasePointer(), this->size());
		}
	}

	FW::HASH CalculateHash() {
		return this->empty() ? NULL : FW::HashData(this->GetBasePointer(), this->size());
	}

	// Simply compare based off sums of bytes
	bool operator<(ByteDataStream& other) {
		return CalculateHash() < other.CalculateHash();
	}
};

// Stores the difference from one ByteDataSteam to the next 
// TODO: Implement
class ByteDataStreamDiff {
public:
	vector<pair<size_t, BYTE>> additions;
	vector<size_t> removals;
	vector<pair<size_t, BYTE>> changes;
};