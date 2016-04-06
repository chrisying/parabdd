#include <cassert>

#include "bdd.h"

using namespace bdd_internal;

Node::Node() { }

Node::Node(bdd::Variable root, Node* branch_true, Node* branch_false) {
	// TODO: look up the node in the table then create a new one
	assert(false);
}
