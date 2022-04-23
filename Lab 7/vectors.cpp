#include <iostream>
#include <string>
#include "vectors.h"

//constructor for an empty Vector
Vector::Vector() {
	this->name = "";
	this->next = nullptr;
	this->size = 0;
	this->elements = nullptr;
}

//constructor for a named Vector
Vector::Vector(std::string name) {
	this->name = name;
	this->next = nullptr;
	std::cout << "Setting vector: " << name << std::endl;
	std::cout << "Enter vector size: ";
	std::cin >> this->size;
	this->elements = new int[this->size];
	for (int i = 0; i < this->size; i++) {
		std::cout << "Enter element " << i << " : ";
		std::cin >> this->elements[i];
	}
}

//destructor for Vector, note that it frees up *elements
Vector::~Vector() {
	std::cout << "Deleted vector: " << this->name << std::endl;
	delete[] this->elements;
}

//constructor for List
List::List() {
	head = nullptr;
	current = nullptr;
}

//destructor for List, it frees up dynamically allocated current vector
List::~List() {
	Vector* old = this->head;

	while (old) {
		this->head = this->head->next;
		delete old;
		old = this->head;
	}
}

void List::add_vector(std::string name) {
	Vector* v = new Vector(name);
	if (!this->head) {
		this->head = v;
		this->current = this->head;
	}
	else {
		this->current->next = v;
		this->current = v;
	}
}

bool List::remove_vector(std::string name) {
	bool removed = false, updated = false;

	Vector* current = this->head;
	Vector* prev = this->head;

	while (current) {
		if (current->name == name) {
			Vector* ptr = current;

			if (current == this->head) {
				this->head = this->head->next;
			}
			prev->next = current->next;
			current = current->next;

			updated = true;
			removed = true;
			delete ptr;
		}

		if (!updated) {
			prev = current;
			current = current->next;
		}

		updated = false;
	}
	return removed;
}

bool List::print_vector(std::string name) {

	Vector* current = this->head;
	bool found = false;

	while (current) {
		if (current->name == name) {
			std::cout << "Vector name: " << current->name << std::endl;
			
			for (size_t i = 0; i < current->size; i++) {
				std::cout << "Element " << i+1 << " for " << current->name << " is " << current->elements[i] << std::endl;
			}

			found = true;
		}
		current = current->next;
	}

	return found;
}

bool List::append_vector(std::string name) {
	bool appended = false;

	Vector* current = this->head;

	while (current) {
		if (current->name == name) {

			int size = 0;

			std::cout << "Enter vector size: ";
			std::cin >> size;

			int* elem = new int[current->size + size];
			for (int i = 0; i < current->size; i++) {
				elem[i] = current->elements[i];
			}

			for (int i = current->size; i < current->size + size; i++) {
				std::cout << "Enter element " << i << " : ";
				std::cin >> elem[i];
			}

			delete[] current->elements;

			current->elements = elem;
			current->size += size;

			appended = true;
		}
		current = current->next;
	}
	return appended;
}