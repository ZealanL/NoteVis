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
			return stream->ReadFromBytes(curIndex, out);
		}
	};

	template <typename T>
	void WriteAsBytes(T object) {
		this->insert(this->end(), (BYTE*)&object, ((BYTE*)&object) + sizeof(T));
	}

	// Returns true if bytes were read, false if out of range
	template <typename T>
	bool ReadFromBytes(int index, T* out) {
		if (index + sizeof(T) >= this->size())
			return false;

		memcpy(out, this->begin()._Ptr + index, sizeof(T));
		return true;
	}

	ReadIterator GetIterator() {
		return ReadIterator(this);
	}
};

