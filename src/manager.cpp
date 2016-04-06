#include <cassert>

#include "bdd.h"

using namespace bdd_internal;

Manager manager;

Manager::Manager() {
	add_nodes();
}

bool Manager::add_nodes() {
	constexpr size_t alloc_count = 64;

	Node (*new_nodes)[alloc_size] = reinterpret_cast<Node(*)[4096]>(new Node[alloc_size * alloc_count]);
	if (new_nodes == nullptr) {
		return false;
	}

	for (size_t i = 0; i < alloc_count; i++) {
		main_nodes.push(&new_nodes[i]);
	}

	return true;
}
