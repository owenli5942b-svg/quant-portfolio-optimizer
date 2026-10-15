#pragma once

#include <iostream>

using namespace std;

struct NodeLL {
	int value;
	NodeLL* next;

	NodeLL(int v) : value(v), next(nullptr) {}
};

class SingleLinkedList {
private:
	NodeLL* head;
	size_t length;

public:
	SingleLinkedList();
	~SingleLinkedList();
	SingleLinkedList(const SingleLinkedList& other);
	SingleLinkedList& operator=(const SingleLinkedList& other);
	SingleLinkedList(SingleLinkedList&& other) noexcept;
	SingleLinkedList& operator=(SingleLinkedList&& other) noexcept;
	void push_front(int value);
	void push_back(int value);
	bool remove(int value);
	void print();
};

SingleLinkedList::SingleLinkedList()
	: head(nullptr), length(0) {
}

SingleLinkedList::~SingleLinkedList() {
	NodeLL* current = head;
	while (current != nullptr) {
		NodeLL* temp = current->next;
		delete current;
		current = temp;
	}
}

void SingleLinkedList::push_front(int value) {
	NodeLL* newNode = new NodeLL(value);
	newNode->next = head;
	head = newNode;
	++length;
}

void SingleLinkedList::push_back(int value) {
	NodeLL* newNode = new NodeLL(value);
	if (head == nullptr)
		head = newNode;
	else {
		NodeLL* current = head;
		while (current->next != nullptr) {
			current = current->next;
		}
		current->next = newNode;
		newNode->next = nullptr;
	}
	++length;
}

bool SingleLinkedList::remove(int value) {
	// Empty case
	if (head == nullptr) {
		return false;
	}

	if (head->value == value) {
		NodeLL* temp = head;
		head = head->next;
		delete temp;
		--length;
		return true;
	}

	NodeLL* current = head;
	while (current->next != nullptr && current->next->value != value) {
		current = current->next;
	}

	// Value not found
	if (current->next == nullptr)
	{
		return false;
	}

	NodeLL* temp = current->next;
	current->next = current->next->next;
	delete temp;
	--length;
	return true;
}

void SingleLinkedList::print() {
	NodeLL* current = head;
	while (current != nullptr) {
		cout << current->value << " ";
		current = current->next;
	}
}

// Rule of Three
SingleLinkedList::SingleLinkedList(const SingleLinkedList& other)
	: head(nullptr), length(0) {
	NodeLL* current = other.head;
	while (current != nullptr) {
		push_back(current->value);
		current = current->next;
	}
}

SingleLinkedList& SingleLinkedList::operator=(const SingleLinkedList& other) {
	// Self assignment guard
	if (this == &other) {
		return *this;
	}

	NodeLL* current = head;
	while (current != nullptr) {
		NodeLL* temp = current->next;
		delete current;
		current = temp;
	}
	head = nullptr;
	length = 0;

	NodeLL* otherCurrent = other.head;
	while (otherCurrent != nullptr) {
		push_back(otherCurrent->value);
		otherCurrent = otherCurrent->next;
	}

	return *this;
}

// Rule of Five
SingleLinkedList::SingleLinkedList(SingleLinkedList&& other) noexcept
	: head(nullptr), length(0) {
	head = other.head;
	length = other.length;
	other.head = nullptr;
	other.length = 0;
}

SingleLinkedList& SingleLinkedList::operator=(SingleLinkedList&& other) noexcept {
	if (this == &other) {
		return *this;
	}

	NodeLL* current = head;
	while (current != nullptr) {
		NodeLL* temp = current->next;
		delete current;
		current = temp;
	}

	head = other.head;
	length = other.length;
	other.head = nullptr;
	other.length = 0;

	return *this;
}
