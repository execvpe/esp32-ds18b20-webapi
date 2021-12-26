#include <cassert>
#include <cstdlib>
#include <cstring>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "DynamicArray.hpp"

// private functions

template <typename E>
void DynamicArray<E>::checkAlloc(size_t requestedSize) {
	if (requestedSize <= size)
		return;

	const size_t oldSize = size;

	size = MAX(size + 4, requestedSize);

	data = reinterpret_cast<E*>(realloc(data, size * sizeof(E)));
	if (data == nullptr)
		abort();

	memset(data + oldSize, 0x0, (size - oldSize) * sizeof(E));
}

// public constructors

template <typename E>
inline DynamicArray<E>::DynamicArray() : DynamicArray(1) {}

template <typename E>
DynamicArray<E>::DynamicArray(size_t initialSize) {
	assert(initialSize >= 1 && "The initial size must be at least 1!");

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
	checkAlloc(idx + 1);

	return data[idx];
}

// public functions

template <typename E>
size_t DynamicArray<E>::getSize() {
	return size;
}
