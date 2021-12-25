#include <cstdlib>
#include <cstring>

#include "DynamicArray.hpp"

// private functions

template <typename E>
void DynamicArray<E>::checkAlloc(size_t requested) {
	if (requested < size)
		return;

	const size_t oldSize = size;

	size << 1;

	data = reinterpret_cast<E*>(realloc(data, size * sizeof(E)));
	if (data == nullptr)
		abort();

	memset(data + oldSize, NULL, oldSize);
}

// public constructors

template <typename E>
inline DynamicArray<E>::DynamicArray() : DynamicArray(1) {}

template <typename E>
DynamicArray<E>::DynamicArray(size_t initialSize) {
	if (initialSize < 1)
		throw -1;

	size = initialSize;

	data = reinterpret_cast<E*>(calloc(size, sizeof(E)));
	if (data == nullptr)
		abort();
	return;
}

// public destructors

template <typename E>
DynamicArray<E>::~DynamicArray() {
	free(static_cast<void*>(data));
}

// public operators

template <typename E>
E& DynamicArray<E>::operator[](size_t idx) {
	checkAlloc(idx);

	return data[idx];
}
