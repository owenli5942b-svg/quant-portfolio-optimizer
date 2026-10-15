#pragma once

#include <iostream>

using namespace std;

struct NodeBT {
	int value;
	NodeBT* left;
	NodeBT* right;
	NodeBT(int v) : value(v), left(nullptr), right(nullptr) {}
	
};

class BinaryTree {
private:
	NodeBT* root;
	size_t length;
	// Recursion function to destory the tree
	void destroy(NodeBT* node) {
		if (node == nullptr) {
			return;
		}
		destroy(node->left);
		destroy(node->right);
		delete node;
	}

public:
	BinaryTree();
	~BinaryTree();
	size_t len();
	void insert(int value);
	bool remove(int value);
};

BinaryTree::BinaryTree()
	: root(nullptr), length(0) {
}

BinaryTree::~BinaryTree() {
	destroy(root);
}

size_t BinaryTree::len() {
	return length;
}

void BinaryTree::insert(int value) {
	NodeBT* current = root;
	while (current != nullptr) {
		if (value < current->value) {
			if (current->left == nullptr) {
				current->left = new NodeBT(value);
				++length;
				break;
			}
			current = current->left;
		}
		else {
			if (current->right == nullptr) {
				current->right = new NodeBT(value);
				++length;
				break;
			}
			current = current->right;
		}
	}
}

bool BinaryTree::remove(int value) {
	NodeBT* current = root;
	NodeBT* parent = nullptr;
	while (current != nullptr) {
		if (value < current->value) {
			if (current->left == nullptr) {
				return false;
			}
			parent = current;
			current = current->left;
		}
		else if (value > current->value) {
			if (current->right == nullptr) {
				return false;
			}
			parent = current;
			current = current->right;
		}
		else {
			// Node found, remove it
			if (parent == nullptr) {
				// Removing the root node
				if (current->left == nullptr && current->right == nullptr) {
					delete current;
					root = nullptr;
					--length;
				}
				else if (current->left == nullptr || current->right == nullptr) {
					root = (current->left != nullptr) ? current->left : current->right;
					delete current;
					--length;
				}
				else {
					// Node has two children, find the in-order successor
					NodeBT* successorParent = current;
					NodeBT* successor = current->right;
					while (successor->left != nullptr) {
						successorParent = successor;
						successor = successor->left;
					}
					current->value = successor->value; // Replace value
					if (successorParent->left == successor) {
						successorParent->left = successor->right;
					}
					else {
						successorParent->right = successor->right;
					}
					delete successor;
					--length;
				}
			}
			else {
				// Removing a non-root node
				if (current->left == nullptr && current->right == nullptr) {
					if (parent->left == current) {
						parent->left = nullptr;
					}
					else {
						parent->right = nullptr;
					}
					delete current;
					--length;
				}
				else if (current->left == nullptr || current->right == nullptr) {
					NodeBT* child = (current->left != nullptr) ? current->left : current->right;
					if (parent->left == current) {
						parent->left = child;
					}
					else {
						parent->right = child;
					}
					delete current;
					--length;
				}
				else {
					// Node has two children, find the in-order successor
					NodeBT* successorParent = current;
					NodeBT* successor = current->right;
					while (successor->left != nullptr) {
						successorParent = successor;
						successor = successor->left;
					}
					current->value = successor->value; // Replace value
					if (successorParent->left == successor) {
						successorParent->left = successor->right;
					}
					else {
						successorParent->right = successor->right;
					}
					delete successor;
					--length;
				}
			}
			return true;
		}
	}
	return false;
}