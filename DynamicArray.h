#pragma once

#include <iostream>

using namespace std;

class DynamicArray {
private:
	int* data;
	size_t length;
	size_t capacity;

public:
	DynamicArray();
	~DynamicArray();
	DynamicArray(const DynamicArray& other);
	DynamicArray(DynamicArray&& other) noexcept;

	void push_back(int value);
	int size();
	bool empty();
	int& operator[](size_t index);
	const int& operator[](size_t index) const; // Trailing const makes sure that no modifications on the dynamic array is allowed
	size_t size() const;
	size_t cap() const;
	DynamicArray& operator=(const DynamicArray& other);
	DynamicArray& operator=(DynamicArray&& other) noexcept;
};

DynamicArray::DynamicArray()
	: data(nullptr), length(0), capacity(0) {
}

DynamicArray::~DynamicArray() {
	delete[] data;
}

void DynamicArray::push_back(int value) {
	if (length == capacity) {
		size_t newCap = (capacity == 0) ? 8 : capacity * 2; // We want to double to avoid allocating memory every single time we push back an element
		int* newData = new int[newCap]; // Address for storing data in a new dynamic list with double the capacity

		for (size_t i = 0; i < length; ++i) {
			newData[i] = data[i]; // newData[i] --> find the next memory address of the element in the dynamic list
		}

		delete[] data;
		data = newData;
		capacity = newCap;
	}
	data[length] = value;
	++length;
}

int DynamicArray::size() {
	return length;
}

bool DynamicArray::empty() {
	return length == 0;
}

int& DynamicArray::operator[](size_t index) {
	return data[index];
}

const int& DynamicArray::operator[](size_t index) const {
	return data[index];
}

size_t DynamicArray::size() const {
	return length;
}

size_t DynamicArray::cap() const {
	return capacity;
}

// The Rule of Three (C++ requires destructor, copy constructor, and copy assignment operator)
DynamicArray::DynamicArray(const DynamicArray& other)
	: data(new int[other.capacity]), length(other.length), capacity(other.capacity) {
	for (size_t i = 0; i < length; ++i)
	{
		data[i] = other.data[i];
	}
}

DynamicArray& DynamicArray::operator=(const DynamicArray& other) {
	if (this == &other) // Self-assignment guard
		return *this;

	int* newData = new int[other.capacity];
	for (size_t i = 0; i < other.length; ++i)
	{
		newData[i] = other.data[i];
	}

	delete[] data;
	data = newData;
	length = other.length;
	capacity = other.capacity;
	return *this;
}

// The Rule of Five (new version grr)
// Move constructor
DynamicArray::DynamicArray(DynamicArray&& other) noexcept
	: data(other.data), length(other.length), capacity(other.capacity) {
	other.data = nullptr;
	other.length = 0;
	other.capacity = 0;
}

DynamicArray& DynamicArray::operator=(DynamicArray&& other) noexcept {
	if (this == &other)
		return *this;

	delete[] data;

	data = other.data;
	length = other.length;
	capacity = other.capacity;

	other.data = nullptr;
	other.length = 0;
	other.capacity = 0;

	return *this;
}