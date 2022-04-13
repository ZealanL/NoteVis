#pragma once

#include "../Framework.h"

// A list with an unchanging size

template <typename T>
struct StaticList {
private:
	T* _data;
	int _size;
public:

	const int& size = _size;

	StaticList() {
		_size = 0;
		_data = NULL;
	}

	StaticList(const T* begin, int size) {
		_size = size;
		_data = (T*)malloc(size * sizeof(T));
		memcpy(_data, begin, GetDataAllocSize());
	}

	StaticList(std::initializer_list<T> data) : StaticList(data.begin(), data.size()) {}
	StaticList(const StaticList& other) : StaticList(other._data, other._size) {}

	T& operator[](int index) {
		IASSERT(index, _size);
		return _data[index];
	}

	const T& operator[](int index) const {
		IASSERT(index, _size);
		return _data[index];
	}


	// size * sizeof(T)
	int GetDataAllocSize() const {
		return _size * sizeof(T);
	}

	bool IsEmpty() const {
		return _size == 0;
	}

	~StaticList() {
		if (_data)
			free(_data);
	}

	// For C++ iterator
	T* begin() const { return IsEmpty() ? NULL : _data; }
	T* end() const { return IsEmpty() ? NULL : _data + GetDataAllocSize(); }
};