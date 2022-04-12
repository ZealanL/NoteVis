#pragma once
#include "../Framework.h"

// Steam of bytes for serialization
class ByteDataSteam : public vector<BYTE> {
public:
	struct ReadIterator {
		ByteDataSteam* stream;
		int curIndex;

		ReadIterator(ByteDataSteam* stream) : stream(stream) {
			curIndex = 0;
		}

		int BytesLeft() {
			return MAX(0, stream->size() - curIndex);
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
	};

	BYTE* GetBasePointer() {
		return this->empty() ? NULL : this->begin()._Ptr;
	}

	template <typename T>
	void WriteAsBytes(T object) {
		this->insert(this->end(), (BYTE*)&object, ((BYTE*)&object) + sizeof(T));
	}

	// Returns true if bytes were read, false if out of range
	template <typename T>
	bool ReadFromBytes(int index, T* out) {
		if (index + sizeof(T) >= this->size())
			return false;

		memcpy(out, GetBasePointer() + index, sizeof(T));
		return true;
	}

	ReadIterator GetIterator() {
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

	void WriteToFileStream(std::ofstream& streamOut) {
		if (!this->empty())
			streamOut.write((char*)GetBasePointer(), this->size());
	}
};

