#include <cassert>

#include "bdd.h"

using namespace bdd_internal;

Node::Node() { }

Node::Node(bdd::Variable root, Node* branch_true, Node* branch_false) : root(root), branch_true(branch_true), branch_false(branch_false) { }
