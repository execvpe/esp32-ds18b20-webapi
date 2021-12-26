#pragma once
#ifndef DYNAMIC_ARRAY_HPP
#define DYNAMIC_ARRAY_HPP

#include <cstddef>

template <typename E>
class DynamicArray {
   private:
	// private variables
	E* data;
	size_t size;

	// private functions
	void checkAlloc(size_t requested);

   public:
	// public constructors
	DynamicArray();
	DynamicArray(size_t initialSize);

	// public destructors
	~DynamicArray();

	// public operators
	E& operator[](size_t idx);

	// public functions

	size_t getSize();
};

#include "DynamicArray.tpp"

#endif	// DYNAMIC_ARRAY_HPP
