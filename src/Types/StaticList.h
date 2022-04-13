#pragma once

#include "../Framework.h"

// A list with an unchanging size

template <typename T>
struct StaticList {
private:
	T* _data = NULL;
	int _size = 0;
public:

	StaticList(const StaticList& other) {
		StaticList(other._data, other._size);
	}

	StaticList(const T* begin, int size) {
		_data = (T*)malloc(size * sizeof(T));
		memcpy(_data, begin, GetDataAllocSize());
	}

	StaticList(std::initializer_list<T> data) {
		StaticList(data.begin(), data.size());
	}

	T& operator[](int index) {
		IASSERT(index, _size);
		return _data[index];
	}

	const T& operator[](int index) const {
		IASSERT(index, _size);
		return _data[index];
	}

	int GetSize() const {
		return _size;
	}

	// GetSize() * sizeof(T)
	int GetDataAllocSize() const {
		return GetSize() * sizeof(T);
	}

	bool IsEmpty() const {
		return _size == 0;
	}

	~StaticList() {
		if (_data)
			free(_data);
	}
};